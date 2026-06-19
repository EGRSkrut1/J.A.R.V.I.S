import 'package:flutter/material.dart';
import 'screens/chat_screen.dart';
import 'screens/videos_screen.dart';
import 'screens/reports_screen.dart';
import 'screens/login_screen.dart';

void main() {
  runApp(const JarvisApp());
}

class JarvisApp extends StatelessWidget {
  const JarvisApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'J.A.R.V.I.S',
      debugShowCheckedModeBanner: false,
      theme: ThemeData.dark().copyWith(
        scaffoldBackgroundColor: Colors.transparent,
        primaryColor: const Color(0xFF00F0FF),
      ),
      initialRoute: '/',
      routes: {
        '/': (context) => const LoginScreen(),
        '/chat': (context) => const ChatScreen(),
        '/videos': (context) => const VideosScreen(),
        '/reports': (context) => const ReportsScreen(),
      },
    );
  }
}