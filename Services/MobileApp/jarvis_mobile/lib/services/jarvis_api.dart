import 'dart:convert';
import 'package:http/http.dart' as http;

class JarvisApi {
  static const String baseUrl = 'http://localhost:8080';

  Future<String> sendMessage(String text) async {
    try {
      final response = await http.post(
        Uri.parse('$baseUrl/chat'),
        headers: {'Content-Type': 'application/json'},
        body: jsonEncode({'message': text}),
      );

      if (response.statusCode == 200) {
        final data = jsonDecode(response.body);
        return data['reply'] ?? 'Ошибка: пустой ответ';
      }
      return 'Ошибка: ${response.statusCode}';
    } catch (e) {
      return 'Ошибка подключения к J.A.R.V.I.S.: $e';
    }
  }

  Future<String> getStatus() async {
    try {
      final response = await http.get(Uri.parse('$baseUrl/status'));
      if (response.statusCode == 200) {
        final data = jsonDecode(response.body);
        return data['status'] ?? 'unknown';
      }
      return 'error';
    } catch (e) {
      return 'offline';
    }
  }

  Future<List<String>> getMessages() async {
    try {
      final response = await http.get(Uri.parse('$baseUrl/messages'));
      if (response.statusCode == 200) {
        final List<dynamic> data = jsonDecode(response.body);
        return data.map((e) => e.toString()).toList();
      }
      return [];
    } catch (e) {
      return [];
    }
  }

  Future<String> getVideo(String filename) async {
    // TODO: реализовать запрос видео с сервера
    return '$baseUrl/videos/$filename';
  }

  Future<List<String>> listVideos() async {
    try {
      final response = await http.get(Uri.parse('$baseUrl/videos/list'));
      if (response.statusCode == 200) {
        final List<dynamic> data = jsonDecode(response.body);
        return data.map((e) => e.toString()).toList();
      }
      return [];
    } catch (e) {
      return [];
    }
  }
}