import 'package:flutter/material.dart';
import 'dart:math';

class AnimatedBackground extends StatefulWidget {
  final Widget child;

  const AnimatedBackground({super.key, required this.child});

  @override
  State<AnimatedBackground> createState() => _AnimatedBackgroundState();
}

class _AnimatedBackgroundState extends State<AnimatedBackground>
    with SingleTickerProviderStateMixin {
  late AnimationController _controller;
  final List<Offset> _particles = [];
  final Random _random = Random();

  @override
  void initState() {
    super.initState();
    _controller = AnimationController(
      vsync: this,
      duration: const Duration(seconds: 20),
    )..repeat(reverse: true);

    for (int i = 0; i < 45; i++) {
      _particles.add(Offset(
        _random.nextDouble(),
        _random.nextDouble(),
      ));
    }
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return AnimatedBuilder(
      animation: _controller,
      builder: (context, child) {
        final screenSize = MediaQuery.of(context).size;
        return Container(
          decoration: BoxDecoration(
            gradient: RadialGradient(
              center: Alignment(
                0.2 + 0.1 * sin(_controller.value * 2 * pi),
                0.3 + 0.1 * cos(_controller.value * 2 * pi),
              ),
              radius: 1.8,
              colors: const [
                Color(0xFF0A1A2E),
                Color(0xFF0D1F35),
                Color(0xFF081624),
                Color(0xFF040C16),
              ],
              stops: [0.0, 0.3, 0.7, 1.0],
            ),
          ),
          child: Stack(
            children: [
              // Дымка
              ...List.generate(4, (index) {
                final phase = index * 0.25 + _controller.value;
                final x = 0.3 + 0.4 * sin(phase * 2 * pi);
                final y = 0.4 + 0.3 * cos(phase * 2 * pi + 0.5);
                final size = 300 + 150 * sin(phase * 2 * pi + 1.2);

                return Positioned(
                  left: x * screenSize.width - size / 2,
                  top: y * screenSize.height - size / 2,
                  child: Container(
                    width: size,
                    height: size,
                    decoration: BoxDecoration(
                      gradient: RadialGradient(
                        center: Alignment.center,
                        radius: 0.6,
                        colors: [
                          const Color(0xFF00F0FF).withOpacity(0.04 + 0.02 * sin(phase * 2 * pi)),
                          const Color(0xFF0066FF).withOpacity(0.02 + 0.015 * cos(phase * 2 * pi)),
                          Colors.transparent,
                        ],
                      ),
                    ),
                  ),
                );
              }),

              // Мерцающие точки
              ..._particles.asMap().entries.map((entry) {
                final idx = entry.key;
                final pos = entry.value;
                final opacity = 0.1 + 0.2 * (0.5 + 0.5 * sin(
                  _controller.value * 2 * pi * (0.3 + idx * 0.05) + idx * 0.7,
                ));

                return Positioned(
                  left: pos.dx * screenSize.width,
                  top: pos.dy * screenSize.height,
                  child: Container(
                    width: 1.5 + idx % 2,
                    height: 1.5 + idx % 2,
                    decoration: BoxDecoration(
                      color: const Color(0xFF00F0FF).withOpacity(opacity),
                      borderRadius: BorderRadius.circular(2),
                      boxShadow: [
                        BoxShadow(
                          color: const Color(0xFF00F0FF).withOpacity(opacity * 0.5),
                          blurRadius: 6 + idx % 3 * 2,
                        ),
                      ],
                    ),
                  ),
                );
              }),

              widget.child,
            ],
          ),
        );
      },
    );
  }
}