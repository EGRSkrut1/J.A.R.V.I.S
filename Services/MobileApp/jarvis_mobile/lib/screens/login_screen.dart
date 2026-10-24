import 'package:flutter/material.dart';

class LoginScreen extends StatefulWidget {
  const LoginScreen({super.key});

  @override
  State<LoginScreen> createState() => _LoginScreenState();
}

class _LoginScreenState extends State<LoginScreen> {
  final TextEditingController _passwordController = TextEditingController();
  String _error = '';

  final Map<String, String> _users = {
    'admin': 'EGEGRSRS',
    'user': '12345',
  };

  void _login() {
    final password = _passwordController.text.trim();
    String? foundUser;

    for (var entry in _users.entries) {
      if (entry.value == password) {
        foundUser = entry.key;
        break;
      }
    }

    if (foundUser != null) {
      Navigator.pushReplacement(
        context,
        MaterialPageRoute(
          builder: (context) => MainScreen(username: foundUser!),
        ),
      );
    } else {
      setState(() => _error = 'Неверный пароль');
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: const Color(0xFF0A1A2E),
      body: Center(
        child: Padding(
          padding: const EdgeInsets.all(32),
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              ShaderMask(
                shaderCallback: (bounds) => const LinearGradient(
                  colors: [Colors.white, Color(0xFF00F0FF)],
                ).createShader(
                  const Rect.fromLTWH(0, 0, 200, 60),
                ),
                child: const Text(
                  'J.A.R.V.I.S',
                  style: TextStyle(
                    fontSize: 32,
                    fontWeight: FontWeight.w700,
                    letterSpacing: 4,
                    color: Colors.white,
                  ),
                ),
              ),
              const SizedBox(height: 40),
              Container(
                decoration: BoxDecoration(
                  gradient: LinearGradient(
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
                padding: const EdgeInsets.all(24),
                child: Column(
                  children: [
                    TextField(
                      controller: _passwordController,
                      obscureText: true,
                      style: const TextStyle(color: Colors.white),
                      decoration: InputDecoration(
                        hintText: 'Введите пароль',
                        hintStyle: TextStyle(
                          color: Colors.white.withOpacity(0.3),
                        ),
                        border: InputBorder.none,
                      ),
                      onSubmitted: (_) => _login(),
                    ),
                    const SizedBox(height: 16),
                    if (_error.isNotEmpty)
                      Text(
                        _error,
                        style: const TextStyle(color: Colors.red),
                      ),
                    const SizedBox(height: 16),
                    GestureDetector(
                      onTap: _login,
                      child: Container(
                        width: double.infinity,
                        padding: const EdgeInsets.symmetric(vertical: 14),
                        decoration: BoxDecoration(
                          gradient: const LinearGradient(
                            colors: [Color(0xFF00F0FF), Color(0xFF0066FF)],
                          ),
                          borderRadius: BorderRadius.circular(30),
                        ),
                        child: const Center(
                          child: Text(
                            'Войти',
                            style: TextStyle(
                              color: Colors.black,
                              fontWeight: FontWeight.w600,
                            ),
                          ),
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

class MainScreen extends StatelessWidget {
  final String username;

  const MainScreen({super.key, required this.username});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: const Color(0xFF0A1A2E),
      body: SafeArea(
        child: Center(
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              ShaderMask(
                shaderCallback: (bounds) => const LinearGradient(
                  colors: [Colors.white, Color(0xFF00F0FF)],
                ).createShader(
                  const Rect.fromLTWH(0, 0, 300, 60),
                ),
                child: const Text(
                  'J.A.R.V.I.S',
                  style: TextStyle(
                    fontSize: 48,
                    fontWeight: FontWeight.w700,
                    letterSpacing: 8,
                    color: Colors.white,
                  ),
                ),
              ),
              const SizedBox(height: 8),
              Text(
                'Добро пожаловать, $username',
                style: const TextStyle(
                  color: Colors.white54,
                  fontSize: 16,
                ),
              ),
              const SizedBox(height: 40),
              _buildMenuButton(
                context,
                icon: Icons.chat,
                label: 'Чат',
                route: '/chat',
              ),
              const SizedBox(height: 16),
              _buildMenuButton(
                context,
                icon: Icons.video_library,
                label: 'Видео',
                route: '/videos',
              ),
              const SizedBox(height: 16),
              _buildMenuButton(
                context,
                icon: Icons.assessment,
                label: 'Отчёты',
                route: '/reports',
              ),
            ],
          ),
        ),
      ),
    );
  }

  Widget _buildMenuButton(BuildContext context, {
    required IconData icon,
    required String label,
    required String route,
  }) {
    return GestureDetector(
      onTap: () => Navigator.pushNamed(context, route),
      child: Container(
        width: 200,
        padding: const EdgeInsets.symmetric(horizontal: 24, vertical: 16),
        decoration: BoxDecoration(
          gradient: LinearGradient(
            colors: [
              Colors.white.withOpacity(0.08),
              Colors.white.withOpacity(0.02),
            ],
          ),
          borderRadius: BorderRadius.circular(60),
          border: Border.all(
            color: const Color(0xFF00F0FF).withOpacity(0.2),
          ),
        ),
        child: Row(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            Icon(icon, color: const Color(0xFF00F0FF)),
            const SizedBox(width: 12),
            Text(
              label,
              style: const TextStyle(
                color: Colors.white70,
                fontSize: 16,
                fontWeight: FontWeight.w600,
              ),
            ),
          ],
        ),
      ),
    );
  }
}