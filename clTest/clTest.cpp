#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <string>

int main() {
	setlocale(LC_ALL, "rus");
	std::string inputPath;
	std::ifstream inputFile("H:\\OutfitME\\outfit_me\\clTest\\x64\\Debug\\input.txt"); //чекнуть разницу между вручную и кодом

	if (!inputFile.is_open()) {
		std::cerr << "Не удалось открыть input.txt!" << std::endl;
		return -1;
	}

	std::getline(inputFile, inputPath);
	inputFile.close();

	std::cout << "Чтение пути изображения: " << inputPath << std::endl;

	// Проверка, что путь не пустой
	if (inputPath.empty()) {
		std::cerr << "Путь к изображению пустой!" << std::endl;
		return -1;
	}

	// Чтение изображения
	cv::Mat image = cv::imread(inputPath);
	if (image.empty()) {
		std::cerr << "Не удалось загрузить изображение по пути: " << inputPath << std::endl;
		return -1;
	}

	std::cout << "Изображение загружено успешно!" << std::endl;

	// Преобразование изображения в HSV
	cv::Mat hsvImage;
	cv::cvtColor(image, hsvImage, cv::COLOR_BGR2HSV);

	cv::Scalar lowerGreen(35, 40, 40);
	cv::Scalar upperGreen(85, 255, 255);
	cv::Mat greenMask;

	// Создание маски для выделения зеленого цвета
	cv::inRange(hsvImage, lowerGreen, upperGreen, greenMask);

	cv::Mat nonGreenImage;
	cv::bitwise_not(greenMask, greenMask);  // Инвертируем маску
	cv::bitwise_and(image, image, nonGreenImage, greenMask);  // Применяем маску

	std::string outputPath = "rofl.png";
	if (cv::imwrite(outputPath, nonGreenImage)) {
		std::cout << "Изображение сохранено успешно: " << outputPath << std::endl;
	}
	else {
		std::cerr << "Ошибка при сохранении изображения!" << std::endl;
		return -1;
	}

	// Запись пути к сохраненному изображению в output.txt
	std::ofstream outputFile("output.txt");
	if (!outputFile.is_open()) {
		std::cerr << "Не удалось открыть output.txt!" << std::endl;
		return -1;
	}
	outputFile << outputPath;
	outputFile.close();

	std::cout << "Путь к изображению записан в output.txt." << std::endl;

	// Для удержания консоли открытой (если программа сразу закрывается)
	system("pause");

	return 0;
}


/*#include <opencv2/opencv.hpp>
#include <opencv2/objdetect.hpp>
#include <iostream>
using namespace cv;
using namespace std;
using namespace samples;
int main() {

	CascadeClassifier faceCascade;
	if (!faceCascade.load(findFile("H:/OutfitME/outfit_me/clTest/x64/Debug/haarcascade_frontalface_default.xml"))) {
		cerr << "Ошибка: не удалось загрузить каскад!" << endl;
		return -1;
	}
	VideoCapture cap(0);
	if (!cap.isOpened()) {
		cerr << "Ошибка: не удалось открыть камеру!" << endl;
		return -1;
	}

	Mat frame;
	while (true) {
		cap.read(frame);
		if (frame.empty()) {
			cerr << "Ошибка: пустой кадр!" << endl;
			break;
		}
		flip(frame, frame, 1);
		Mat grayFrame;
		cvtColor(frame, grayFrame, COLOR_BGR2GRAY);
		equalizeHist(grayFrame, grayFrame); 

		vector<Rect> faces;
		faceCascade.detectMultiScale(grayFrame, faces, 1.1, 3, 0, Size(30, 30));

		for (const auto& face : faces) {
			rectangle(frame, face, Scalar(59, 89, 89), 2);
		}
		imshow("Face Tracking", frame);
		if (waitKey(30) == 'q') {
			break;
		}
	}

	return 0;
}

/*#include "stdafx.h"

#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main(int argc, char* argv[])
{
	VideoCapture cap(0);
	if (cap.isOpened() == false)
	{
		cout << "Cannot open the video camera" << endl;
		cin.get(); 
		return -1;
	}

	double dWidth = cap.get(CAP_PROP_FRAME_WIDTH); 
	double dHeight = cap.get(CAP_PROP_FRAME_HEIGHT);

	cout << "Resolution of the video : " << dWidth << " x " << dHeight << endl;

	string window_name = "My Camera";
	namedWindow(window_name); 

	while (true)
	{
		Mat frame;
		bool bSuccess = cap.read(frame); 
		if (bSuccess == false)
		{
			cout << "Video camera is disconnected" << endl;
			cin.get();
			break;
		}
		Mat inverse;
		flip(frame, inverse, 1);
		Mat brighNessFrame;
		int brighNess = 10;
		frame.convertTo(brighNessFrame, -1, 1, brighNess);
		if (waitKey(10) == 49) {
			brighNess--;
			frame.convertTo(brighNessFrame, -1, 1, brighNess);
		}
		if (waitKey(10) == 50) {
			brighNess++;
			frame.convertTo(brighNessFrame, -1, 1, brighNess);
		}
		flip(frame, brighNessFrame, 1);
		imshow(window_name, brighNessFrame);

		if (waitKey(10) == 27)
		{
			cout << "Esc key is pressed by user. Stoppig the video" << endl;
			break;
		}
	}

	return 0;
}*/