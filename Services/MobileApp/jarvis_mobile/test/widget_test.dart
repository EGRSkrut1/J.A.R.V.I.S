import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:jarvis_mobile/main.dart';

void main() {
  testWidgets('J.A.R.V.I.S app smoke test', (WidgetTester tester) async {
    await tester.pumpWidget(const JarvisApp());
    expect(find.text('J.A.R.V.I.S'), findsOneWidget);
  });
}