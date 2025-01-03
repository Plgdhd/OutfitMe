import 'dart:core';
import 'dart:io';
import 'package:flutter/material.dart';
import 'dart:async';

class ResultScreen extends StatefulWidget {
  const ResultScreen({super.key});
  @override
  _ResultScreenState createState() => _ResultScreenState();
}

class _ResultScreenState extends State<ResultScreen> {
  bool isProcessing = true;

  @override
  void initState() {
    super.initState();
    _simulateProcessing();
  }

  Future<void> _simulateProcessing() async {
    await Future.delayed(const Duration(seconds: 3));
    setState(() {
      isProcessing = false;
    });
  }

  Future<void> downloadFile() async {
    const filePath = 'result_with_selected_item.jpg';

    try {
      final directory =
          Directory("${Platform.environment['USERPROFILE']}\\Downloads");
      if (!directory.existsSync()) {
        print("Папка Загрузки не найдена.");
        return;
      }

      final file = File(filePath);
      if (!file.existsSync()) {
        print("Исходный файл не найден: $filePath");
        return;
      }

      final savePath = '${directory.path}\\${file.uri.pathSegments.last}';
      await file.copy(savePath);

      if (mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          const SnackBar(
            content: Text(
              "Изображение успешно скачано!",
              style: TextStyle(fontSize: 16, fontWeight: FontWeight.bold),
            ),
            backgroundColor: Colors.green,
            duration: Duration(seconds: 2),
          ),
        );
      }

      print('Файл успешно скачан в: $savePath');
    } catch (e) {
      print('Ошибка при скачивании файла: $e');
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        backgroundColor: const Color.fromRGBO(255, 69, 96, 1),
        title: const Text(
          "OutfitMe",
          style: TextStyle(
              fontFamily: 'Astro',
              fontWeight: FontWeight.bold,
              color: Colors.white,
              fontSize: 25),
        ),
        centerTitle: true,
        elevation: 4,
      ),
      body: Center(
        child: isProcessing
            ? const Column(
                mainAxisAlignment: MainAxisAlignment.center,
                children: [
                  CircularProgressIndicator(
                    color: Color.fromRGBO(255, 69, 96, 1),
                    strokeWidth: 6,
                  ),
                  SizedBox(height: 30),
                  Text(
                    "Обрабатываем ваше изображение...",
                    style: TextStyle(
                        fontFamily: 'Roboto',
                        fontSize: 18,
                        fontWeight: FontWeight.bold,
                        color: Color.fromRGBO(255, 69, 96, 1)),
                    textAlign: TextAlign.center,
                  ),
                ],
              )
            : Column(
                mainAxisAlignment: MainAxisAlignment.center,
                children: [
                  const Spacer(flex: 2),
                  const Text(
                    "Готово!",
                    style: TextStyle(
                        fontSize: 50,
                        fontWeight: FontWeight.w900,
                        color: Color.fromRGBO(255, 69, 96, 1)),
                  ),
                  const SizedBox(height: 20),
                  Container(
                    height: 150,
                    width: 150,
                    decoration: BoxDecoration(
                      color: const Color.fromRGBO(245, 245, 245, 1),
                      borderRadius: BorderRadius.circular(75),
                      boxShadow: const [
                        BoxShadow(
                          color: Colors.black26,
                          blurRadius: 12,
                          spreadRadius: 6,
                          offset: Offset(0, 6),
                        ),
                      ],
                    ),
                    child: const Icon(
                      Icons.check_circle,
                      size: 120,
                      color: Color.fromRGBO(255, 69, 96, 1),
                    ),
                  ),
                  const SizedBox(height: 40),
                  ElevatedButton(
                    onPressed: () async {
                      await downloadFile();
                    },
                    style: ButtonStyle(
                      backgroundColor: MaterialStateProperty.all<Color>(
                        const Color.fromRGBO(255, 69, 96, 1),
                      ),
                      padding: MaterialStateProperty.all<EdgeInsets>(
                          const EdgeInsets.symmetric(
                              horizontal: 40, vertical: 15)),
                      shape: MaterialStateProperty.all<RoundedRectangleBorder>(
                        RoundedRectangleBorder(
                          borderRadius: BorderRadius.circular(30),
                        ),
                      ),
                      elevation: MaterialStateProperty.all(8),
                      shadowColor: MaterialStateProperty.all(
                          const Color.fromRGBO(255, 69, 96, 0.5)),
                    ),
                    child: const Text(
                      "Сохранить изображение",
                      style: TextStyle(
                          color: Colors.white,
                          fontSize: 18,
                          fontWeight: FontWeight.bold),
                    ),
                  ),
                  const Spacer(flex: 3),
                ],
              ),
      ),
    );
  }
}
