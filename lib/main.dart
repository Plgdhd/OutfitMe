import 'dart:core';
import 'package:scroll_snap_list/scroll_snap_list.dart';
import 'package:flutter/material.dart';

void main() {
  runApp(MainApp());
}

class MainApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      debugShowCheckedModeBanner: false,
      home: ShowCase(),
    );
  }
}

class ShowCase extends StatefulWidget {
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
  int get _itemCount => 2000;
  int currentIndex = 500;
  Widget _buildItemList(BuildContext context, int index) {
    int adjustedIndex = index % images.length;
    return Container(
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
        backgroundColor: const Color.fromRGBO(245, 28, 86, 1),
        title: const Text(
          "OutfitMe",
          style: TextStyle(fontWeight: FontWeight.bold, color: Colors.white),
        ),
        centerTitle: true,
        leading: const Padding(
          padding: EdgeInsets.only(left: 20),
          child: Image(
            image: AssetImage('assets/images/fitRed.jpg'),
          ),
        ),
      ),
      body: Column(
        children: [
          Container(
            margin: EdgeInsets.only(top: 10),
            child: const Text("Выберите элемент одежды:",
                style: TextStyle(fontWeight: FontWeight.bold, fontSize: 25)),
          ),
          Container(
            width: 450,
            height: 300,
            /* decoration: BoxDecoration(
                border: Border.all(
              width: 4,
              color: Colors.yellow,
            )),*/
            child: Column(
              children: [
                Expanded(
                  child: ScrollSnapList(
                    itemBuilder: _buildItemList,
                    itemSize: 220,
                    dynamicItemOpacity: 0.3,
                    dynamicItemSize: true,
                    itemCount: _itemCount,
                    onItemFocus: (index) {
                      setState(() {
                        currentIndex = index % images.length;
                      });
                      print("Фокус на элементе: $index");
                    },
                    initialIndex: 500,
                  ),
                ),
              ],
            ),
          ),
        ],
      ),
    );
  }
}
