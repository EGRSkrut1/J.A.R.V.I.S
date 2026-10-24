import 'package:flutter/material.dart';
import 'package:http/http.dart' as http;
import 'dart:convert';
import 'dart:async';
import '../widgets/animated_background.dart';

class ChatScreen extends StatefulWidget {
  const ChatScreen({super.key});

  @override
  State<ChatScreen> createState() => _ChatScreenState();
}

class _ChatScreenState extends State<ChatScreen> {
  final TextEditingController _controller = TextEditingController();
  final List<Map<String, dynamic>> _messages = [];
  bool _isLoading = false;
  String _status = 'idle';
  Timer? _timer;

  @override
  void initState() {
    super.initState();
    _loadMessages();
    _pollStatus();
    _timer = Timer.periodic(const Duration(seconds: 1), (timer) {
      _loadMessages();
    });
  }

  @override
  void dispose() {
    _timer?.cancel();
    super.dispose();
  }

  Color get _statusColor {
    switch (_status) {
      case 'processing':
        return const Color(0xFFFFAA00);
      case 'error':
      case 'offline':
        return Colors.red;
      default:
        return Colors.green;
    }
  }

  Future<void> _loadMessages() async {
    try {
      final response = await http.get(
        Uri.parse('http://localhost:8080/messages'),
      );
      if (response.statusCode == 200) {
        final List<dynamic> data = jsonDecode(response.body);
        setState(() {
          _messages.clear();
          for (var msg in data) {
            final String text = msg.toString();
            final bool isUser = text.startsWith('Вы:');
            final String content = isUser
                ? text.substring(3).trim()
                : text.substring(9).trim();
            _messages.add({'text': content, 'isUser': isUser});
          }
        });
      } else {
        setState(() => _status = 'error');
      }
    } catch (e) {
      setState(() => _status = 'offline');
    }
  }

  Future<void> _pollStatus() async {
    while (true) {
      try {
        final response = await http.get(
          Uri.parse('http://localhost:8080/status'),
        );
        if (response.statusCode == 200) {
          final data = jsonDecode(response.body);
          if (mounted) {
            setState(() => _status = data['status'] ?? 'idle');
          }
        } else {
          if (mounted) setState(() => _status = 'error');
        }
      } catch (e) {
        if (mounted) setState(() => _status = 'offline');
      }
      await Future.delayed(const Duration(seconds: 2));
    }
  }

  Future<void> _sendMessage() async {
    final text = _controller.text.trim();
    if (text.isEmpty || _isLoading) return;

    setState(() {
      _messages.add({'text': text, 'isUser': true});
      _controller.clear();
      _isLoading = true;
    });

    try {
      final response = await http.post(
        Uri.parse('http://localhost:8080/chat'),
        headers: {'Content-Type': 'application/json'},
        body: jsonEncode({'message': text}),
      );

      if (response.statusCode == 200) {
        final data = jsonDecode(response.body);
        if (mounted) {
          setState(() {
            _messages.add({'text': data['reply'] ?? 'Нет ответа', 'isUser': false});
          });
        }
      } else {
        if (mounted) {
          setState(() {
            _messages.add({'text': 'Ошибка: ${response.statusCode}', 'isUser': false});
          });
        }
      }
    } catch (e) {
      if (mounted) {
        setState(() {
          _messages.add({'text': 'Ошибка подключения к J.A.R.V.I.S.', 'isUser': false});
        });
      }
    }

    if (mounted) {
      setState(() => _isLoading = false);
    }
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
                    Container(
                      width: 12,
                      height: 12,
                      decoration: BoxDecoration(
                        shape: BoxShape.circle,
                        color: _statusColor,
                        boxShadow: [
                          BoxShadow(
                            color: _statusColor.withOpacity(0.5),
                            blurRadius: 12,
                          ),
                        ],
                      ),
                    ),
                    const SizedBox(width: 12),
                    Expanded(
                      child: ShaderMask(
                        shaderCallback: (bounds) => const LinearGradient(
                          colors: [Colors.white, Color(0xFF00F0FF)],
                        ).createShader(bounds),
                        child: const Text(
                          'J.A.R.V.I.S',
                          style: TextStyle(
                            fontSize: 20,
                            fontWeight: FontWeight.w500,
                            letterSpacing: 4,
                            color: Colors.white,
                          ),
                        ),
                      ),
                    ),
                    IconButton(
                      icon: const Icon(Icons.refresh, color: Color(0xFF00F0FF)),
                      onPressed: _loadMessages,
                    ),
                  ],
                ),
              ),

              // Chat
              Expanded(
                child: ListView.builder(
                  padding: const EdgeInsets.all(16),
                  itemCount: _messages.length,
                  itemBuilder: (context, index) {
                    final msg = _messages[index];
                    final bool isUser = msg['isUser'] as bool;
                    final String text = msg['text'] as String;

                    return AnimatedContainer(
                      duration: const Duration(milliseconds: 300),
                      margin: const EdgeInsets.symmetric(vertical: 4),
                      alignment: isUser ? Alignment.centerRight : Alignment.centerLeft,
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
                              Colors.white.withOpacity(isUser ? 0.12 : 0.05),
                              Colors.white.withOpacity(isUser ? 0.04 : 0.01),
                            ],
                          ),
                          borderRadius: BorderRadius.circular(20),
                          border: Border.all(
                            color: const Color(0xFF00F0FF).withOpacity(isUser ? 0.3 : 0.1),
                            width: 1,
                          ),
                        ),
                        child: Text(
                          text,
                          style: TextStyle(
                            color: isUser ? Colors.white : Colors.white70,
                            fontSize: 15,
                          ),
                        ),
                      ),
                    );
                  },
                ),
              ),

              // Loading
              if (_isLoading)
                Padding(
                  padding: const EdgeInsets.all(8),
                  child: Row(
                    mainAxisAlignment: MainAxisAlignment.center,
                    children: [
                      const SizedBox(
                        width: 20,
                        height: 20,
                        child: CircularProgressIndicator(
                          strokeWidth: 2,
                          color: Color(0xFF00F0FF),
                        ),
                      ),
                      const SizedBox(width: 12),
                      const Text(
                        'Джарвис печатает...',
                        style: TextStyle(color: Colors.white70),
                      ),
                    ],
                  ),
                ),

              // Input
              Container(
                padding: const EdgeInsets.all(16),
                decoration: BoxDecoration(
                  border: Border(
                    top: BorderSide(
                      color: const Color(0xFF00F0FF).withOpacity(0.1),
                    ),
                  ),
                ),
                child: Row(
                  children: [
                    Expanded(
                      child: Container(
                        decoration: BoxDecoration(
                          color: Colors.white.withOpacity(0.05),
                          borderRadius: BorderRadius.circular(24),
                          border: Border.all(
                            color: const Color(0xFF00F0FF).withOpacity(0.2),
                          ),
                        ),
                        child: TextField(
                          controller: _controller,
                          style: const TextStyle(color: Colors.white),
                          decoration: InputDecoration(
                            hintText: 'Сообщение...',
                            hintStyle: TextStyle(
                              color: Colors.white.withOpacity(0.3),
                            ),
                            border: InputBorder.none,
                            contentPadding: const EdgeInsets.symmetric(
                              horizontal: 18,
                              vertical: 12,
                            ),
                          ),
                          onSubmitted: (_) => _sendMessage(),
                        ),
                      ),
                    ),
                    const SizedBox(width: 12),
                    GestureDetector(
                      onTap: _isLoading ? () {} : _sendMessage,
                      child: Container(
                        padding: const EdgeInsets.all(14),
                        decoration: BoxDecoration(
                          gradient: const LinearGradient(
                            colors: [Color(0xFF00F0FF), Color(0xFF0066FF)],
                          ),
                          borderRadius: BorderRadius.circular(30),
                          boxShadow: [
                            BoxShadow(
                              color: const Color(0xFF00F0FF).withOpacity(0.3),
                              blurRadius: 20,
                            ),
                          ],
                        ),
                        child: const Icon(
                          Icons.send,
                          color: Colors.black,
                          size: 22,
                        ),
                      ),
                    ),
                  ],
                ),
              ),
            ],
          ),
        ),
      ),
    );
  }
}