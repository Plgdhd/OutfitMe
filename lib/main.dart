import 'dart:core';
import 'dart:io';
import 'package:scroll_snap_list/scroll_snap_list.dart';
import 'package:flutter/material.dart';
import 'package:image_picker/image_picker.dart';

void main() {
  runApp(const MainApp());
}

class MainApp extends StatefulWidget {
  const MainApp({super.key});

  @override
  _MainAppState createState() => _MainAppState();
}

class _MainAppState extends State<MainApp> {
  bool _isLoading = true;
  bool _isVisible = false;
  @override
  void initState() {
    super.initState();
    _simulateLoading();
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      debugShowCheckedModeBanner: false,
      home: Scaffold(
        body: Stack(
          children: [
            AnimatedOpacity(
              opacity: _isVisible ? 1.0 : 0.0,
              duration: const Duration(seconds: 1),
              child: const ShowCase(),
            ),
            if (_isLoading)
              const Align(
                alignment: Alignment.center,
                child: Column(
                  children: [
                    SizedBox(
                      height: 100,
                    ),
                    Text(
                      "OutfitMe",
                      style: TextStyle(
                        fontSize: 80,
                        color: Color.fromRGBO(245, 28, 86, 1),
                        fontWeight: FontWeight.bold,
                      ),
                    ),
                    SizedBox(
                      height: 250,
                    ),
                    CircularProgressIndicator(),
                  ],
                ),
              ),
          ],
        ),
      ),
    );
  }

  void _simulateLoading() {
    Future.delayed(const Duration(seconds: 4), () {
      setState(() {
        _isLoading = false;
        _isVisible = true;
      });
      _navigateToMainScreen();
    });
  }

  void _navigateToMainScreen() {
    Navigator.pushReplacement(
      context,
      PageRouteBuilder(
        pageBuilder: (context, animation, secondaryAnimation) =>
        const ShowCase(),
        transitionsBuilder: (context, animation, secondaryAnimation, child) {
          const begin = 0.0;
          const end = 1.0;
          const curve = Curves.easeInOut;
          var opacityAnimation = Tween<double>(begin: begin, end: end).animate(
            CurvedAnimation(
              parent: animation,
              curve: curve,
            ),
          );

          return FadeTransition(
            opacity: opacityAnimation,
            child: child,
          );
        },
        transitionDuration: const Duration(seconds: 2),
      ),
    );
    setState(() {
      _isLoading = false;
    });
  }
}

class ShowCase extends StatefulWidget {
  const ShowCase({super.key});

  @override
  _ShowCaseOfImages createState() => _ShowCaseOfImages();
}

class _ShowCaseOfImages extends State<ShowCase> {
  List<String> images = [
    "assets/images/pivo.png",
    "assets/images/MegaShoes.png",
    "assets/images/fitGrey.webp",
    "assets/images/fitRed.jpg"
  ];
  bool isPressed = false;
  int get _itemCount => 500;
  int currentIndex = 250;
  String textPhoto = "Выбрать фото";
  File? _selectedImage;

  Widget _buildItemList(BuildContext context, int index) {
    int adjustedIndex = index % images.length;
    return SizedBox(
      width: 220,
      /*decoration: BoxDecoration(
        border: Border.all(
          width: 4,
          color: Colors.black,
        ),
      ),*/
      child: Column(
        children: [
          Container(
            margin: const EdgeInsets.only(top: 20),
            /*decoration: BoxDecoration(
              border: Border.all(width: 3, color: Colors.red),
              //borderRadius: BorderRadius.circular(10)
            ),*/
            width: 250,
            height: 250,
            alignment: Alignment.center,
            child: Image.asset(
              images[adjustedIndex],
            ),
          ),
        ],
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        elevation: 4,
        backgroundColor: const Color.fromRGBO(245, 28, 86, 1),
        flexibleSpace: Align(
          alignment: Alignment.bottomLeft,
          child: Padding(
            padding: const EdgeInsets.only(left: 20, bottom: 10),
            child: ClipRRect(
              borderRadius: BorderRadius.circular(50),
              child: Container(
                width: 50,
                height: 50,
                decoration: const BoxDecoration(
                  shape: BoxShape.circle,
                ),
                child: const Image(
                  image: AssetImage('assets/images/fitRed.jpg'),
                  fit: BoxFit.cover,
                ),
              ),
            ),
          ),
        ),
        title: const Text(
          "OutfitME",
          style: TextStyle(
              fontWeight: FontWeight.bold, color: Colors.white, fontSize: 35),
        ),
        centerTitle: true,
      ),
      body: Column(
        children: [
          Container(
            margin: const EdgeInsets.only(top: 40),
            child: const Text("Выберите элемент одежды:",
                style: TextStyle(fontWeight: FontWeight.w900, fontSize: 26)),
          ),
          SizedBox(
            width: 450,
            height: 300,
            /* decoration: BoxDecoration(
                border: Border.all(
              width: 4,
              color: Colors.yellow,
            )),*/
            child: Row(
              children: [
                Expanded(
                  child: ScrollSnapList(
                    itemBuilder: _buildItemList,
                    itemSize: 220,
                    dynamicItemOpacity: 0.4,
                    dynamicItemSize: true,
                    itemCount: _itemCount,
                    onItemFocus: (index) {
                      setState(() {
                        currentIndex = index % images.length;
                      });
                      print("Фокус на элементе: $index");
                    },
                    initialIndex: 250,
                  ),
                ),
              ],
            ),
          ),
          Center(
            child: ElevatedButton(
              onPressed: () {
                //передача изображения в с++ или файл хз
              },
              style: const ButtonStyle(
                backgroundColor: WidgetStatePropertyAll<Color?>(
                  Color.fromRGBO(245, 28, 86, 1),
                ),
              ),
              child: const Text("Примерить",
                  style: TextStyle(
                      color: Colors.white,
                      fontSize: 30,
                      fontWeight: FontWeight.bold)),
            ),
          ),
          Container(
            margin: const EdgeInsets.only(top: 30),
            height: 200,
            width: 200,
            decoration: BoxDecoration(
                border: Border.all(
                  width: 3,
                  color: const Color.fromRGBO(245, 28, 86, 1),
                ),
                borderRadius: BorderRadius.circular(23)),
            child: ClipRRect(
              borderRadius: BorderRadius.circular(20),
              child: Center(
                child: _selectedImage != null
                    ? Image.file(
                  _selectedImage!,
                  fit: BoxFit.cover,
                  width: double.infinity,
                  height: double.infinity,
                )
                    : const Text("Ваше фото тута",
                    style: TextStyle(letterSpacing: 5)),
              ),
            ),
          ),
          Container(
            margin: const EdgeInsets.only(top: 30),
            width: 350,
            height: 70,
            child: ElevatedButton(
              onPressed: () {
                pickImageFromGallery();
                Text(textPhoto,
                    style: TextStyle(
                      fontSize: 40,
                      color: Color.fromRGBO(255, 255, 255, 0.26),
                      fontWeight: FontWeight.bold,
                    ));
                isPressed = true;
              },
              style: const ButtonStyle(
                backgroundColor: WidgetStatePropertyAll(
                  Color.fromRGBO(245, 28, 86, 1),
                ),
              ),
              child: Text(
                textPhoto,
                style: TextStyle(
                  fontSize: 40,
                  color: isPressed
                      ? Color.fromRGBO(255, 255, 255, 0.579)
                      : Color.fromRGBO(255, 255, 255, 1),
                  fontWeight: FontWeight.bold,
                ),
                maxLines: 1,
                overflow: TextOverflow.ellipsis,
              ),
            ),
          ),
        ],
      ),
    );
  }

  Future pickImageFromGallery() async {
    final gainedImage =
    await ImagePicker().pickImage(source: ImageSource.gallery);

    setState(() {
      _selectedImage = File(gainedImage!.path);
      textPhoto = gainedImage.name;
    });
  }
}