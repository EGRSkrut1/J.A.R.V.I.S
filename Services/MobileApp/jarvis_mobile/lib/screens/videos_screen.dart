import 'package:flutter/material.dart';
import 'package:video_player/video_player.dart';
import 'package:http/http.dart' as http;
import 'dart:convert';
import '../widgets/animated_background.dart';

class VideosScreen extends StatefulWidget {
  const VideosScreen({super.key});

  @override
  State<VideosScreen> createState() => _VideosScreenState();
}

class _VideosScreenState extends State<VideosScreen> {
  List<String> _videos = [];
  bool _isLoading = true;
  VideoPlayerController? _controller;
  bool _isPlaying = false;
  int _currentVideoIndex = -1;

  @override
  void initState() {
    super.initState();
    _loadVideos();
  }

  @override
  void dispose() {
    _controller?.dispose();
    super.dispose();
  }

  Future<void> _loadVideos() async {
    setState(() => _isLoading = true);
    try {
      final response = await http.get(
        Uri.parse('http://localhost:8080/videos/list'),
      );
      if (response.statusCode == 200) {
        final data = jsonDecode(response.body);
        setState(() => _videos = List<String>.from(data));
      }
    } catch (e) {}
    setState(() => _isLoading = false);
  }

  Future<void> _playVideo(String filename, int index) async {
    final url = 'http://localhost:8080/videos/$filename';
    _controller?.dispose();
    final controller = VideoPlayerController.network(url);
    await controller.initialize();
    setState(() {
      _controller = controller;
      _currentVideoIndex = index;
      _isPlaying = true;
    });
    await controller.play();
  }

  @override
  Widget build(BuildContext context) {
    return AnimatedBackground(
      child: Scaffold(
        backgroundColor: Colors.transparent,
        body: SafeArea(
          child: Column(
            children: [
              // Header
              Container(
                padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 12),
                decoration: BoxDecoration(
                  border: Border(
                    bottom: BorderSide(
                      color: const Color(0xFF00F0FF).withOpacity(0.1),
                    ),
                  ),
                ),
                child: Row(
                  children: [
                    IconButton(
                      icon: const Icon(Icons.arrow_back, color: Color(0xFF00F0FF)),
                      onPressed: () => Navigator.pop(context),
                    ),
                    Expanded(
                      child: ShaderMask(
                        shaderCallback: (bounds) => const LinearGradient(
                          colors: [Colors.white, Color(0xFF00F0FF)],
                        ).createShader(bounds),
                        child: const Text(
                          'Видео',
                          style: TextStyle(
                            fontSize: 20,
                            fontWeight: FontWeight.w500,
                            letterSpacing: 2,
                            color: Colors.white,
                          ),
                        ),
                      ),
                    ),
                    IconButton(
                      icon: const Icon(Icons.refresh, color: Color(0xFF00F0FF)),
                      onPressed: _loadVideos,
                    ),
                  ],
                ),
              ),

              // Video player
              if (_controller != null)
                Padding(
                  padding: const EdgeInsets.all(16),
                  child: Container(
                    decoration: BoxDecoration(
                      gradient: LinearGradient(
                        begin: Alignment.topLeft,
                        end: Alignment.bottomRight,
                        colors: [
                          Colors.white.withOpacity(0.08),
                          Colors.white.withOpacity(0.02),
                        ],
                      ),
                      borderRadius: BorderRadius.circular(20),
                      border: Border.all(
                        color: const Color(0xFF00F0FF).withOpacity(0.2),
                      ),
                    ),
                    child: Column(
                      children: [
                        AspectRatio(
                          aspectRatio: _controller!.value.aspectRatio,
                          child: VideoPlayer(_controller!),
                        ),
                        const SizedBox(height: 8),
                        VideoProgressIndicator(_controller!, allowScrubbing: true),
                        const SizedBox(height: 8),
                        Row(
                          mainAxisAlignment: MainAxisAlignment.center,
                          children: [
                            IconButton(
                              icon: Icon(
                                _isPlaying ? Icons.pause : Icons.play_arrow,
                                color: const Color(0xFF00F0FF),
                              ),
                              onPressed: () {
                                if (_controller!.value.isPlaying) {
                                  _controller!.pause();
                                  setState(() => _isPlaying = false);
                                } else {
                                  _controller!.play();
                                  setState(() => _isPlaying = true);
                                }
                              },
                            ),
                          ],
                        ),
                      ],
                    ),
                  ),
                ),

              // Video list
              Expanded(
                child: _isLoading
                    ? const Center(
                        child: CircularProgressIndicator(
                          color: Color(0xFF00F0FF),
                        ),
                      )
                    : _videos.isEmpty
                        ? Center(
                            child: Column(
                              mainAxisAlignment: MainAxisAlignment.center,
                              children: [
                                const Text(
                                  'Видео не найдены',
                                  style: TextStyle(
                                    color: Colors.white38,
                                    fontSize: 16,
                                  ),
                                ),
                                const SizedBox(height: 16),
                                GestureDetector(
                                  onTap: _loadVideos,
                                  child: Container(
                                    padding: const EdgeInsets.symmetric(
                                      horizontal: 24,
                                      vertical: 14,
                                    ),
                                    decoration: BoxDecoration(
                                      gradient: const LinearGradient(
                                        colors: [Color(0xFF00F0FF), Color(0xFF0066FF)],
                                      ),
                                      borderRadius: BorderRadius.circular(60),
                                      boxShadow: [
                                        BoxShadow(
                                          color: const Color(0xFF00F0FF).withOpacity(0.3),
                                          blurRadius: 20,
                                        ),
                                      ],
                                    ),
                                    child: const Text(
                                      'Обновить',
                                      style: TextStyle(
                                        color: Colors.black,
                                        fontWeight: FontWeight.w600,
                                      ),
                                    ),
                                  ),
                                ),
                              ],
                            ),
                          )
                        : ListView.builder(
                            padding: const EdgeInsets.all(16),
                            itemCount: _videos.length,
                            itemBuilder: (context, index) {
                              final filename = _videos[index];
                              final isActive = _currentVideoIndex == index;

                              return Padding(
                                padding: const EdgeInsets.only(bottom: 12),
                                child: GestureDetector(
                                  onTap: () => _playVideo(filename, index),
                                  child: Container(
                                    padding: const EdgeInsets.symmetric(
                                      horizontal: 16,
                                      vertical: 12,
                                    ),
                                    decoration: BoxDecoration(
                                      gradient: LinearGradient(
                                        begin: Alignment.topLeft,
                                        end: Alignment.bottomRight,
                                        colors: [
                                          Colors.white.withOpacity(isActive ? 0.12 : 0.05),
                                          Colors.white.withOpacity(isActive ? 0.04 : 0.01),
                                        ],
                                      ),
                                      borderRadius: BorderRadius.circular(20),
                                      border: Border.all(
                                        color: isActive
                                            ? const Color(0xFF00F0FF)
                                            : const Color(0xFF00F0FF).withOpacity(0.1),
                                        width: isActive ? 2 : 1,
                                      ),
                                    ),
                                    child: Row(
                                      children: [
                                        Container(
                                          width: 40,
                                          height: 40,
                                          decoration: BoxDecoration(
                                            color: const Color(0xFF00F0FF)
                                                .withOpacity(0.1),
                                            borderRadius: BorderRadius.circular(8),
                                          ),
                                          child: Icon(
                                            isActive ? Icons.play_arrow : Icons.video_file,
                                            color: const Color(0xFF00F0FF),
                                          ),
                                        ),
                                        const SizedBox(width: 12),
                                        Expanded(
                                          child: Text(
                                            filename,
                                            style: TextStyle(
                                              color: isActive
                                                  ? const Color(0xFF00F0FF)
                                                  : Colors.white70,
                                              fontWeight: isActive
                                                  ? FontWeight.w600
                                                  : FontWeight.normal,
                                            ),
                                          ),
                                        ),
                                        if (isActive)
                                          Container(
                                            padding: const EdgeInsets.symmetric(
                                              horizontal: 8,
                                              vertical: 4,
                                            ),
                                            decoration: BoxDecoration(
                                              color: const Color(0xFF00F0FF)
                                                  .withOpacity(0.2),
                                              borderRadius: BorderRadius.circular(12),
                                            ),
                                            child: const Text(
                                              '▶',
                                              style: TextStyle(
                                                color: Color(0xFF00F0FF),
                                                fontSize: 12,
                                              ),
                                            ),
                                          ),
                                      ],
                                    ),
                                  ),
                                ),
                              );
                            },
                          ),
              ),
            ],
          ),
        ),
      ),
    );
  }
}