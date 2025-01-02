#include <opencv2/opencv.hpp>
#include <iostream>
#include <opencv2/dnn.hpp>
#include <string>
#include <vector>

using namespace cv;
using namespace std;
using namespace dnn;

vector<Point> detectBodyKeypoints(const Mat& person);
void overlayImage(const Mat& background, const Mat& foreground, Mat& output, Point2i location, Size tshirtSize);

// Функция наложения изображения
void overlayImage(const Mat& background, const Mat& foreground, Mat& output, Point2i location, Size tshirtSize)
{
	if (background.empty() || foreground.empty()) {
		cerr << "Пустые входные изображения" << endl;
		return;
	}

	if (foreground.channels() != 4) {
		cerr << "Изображение футболки должно иметь 4 канала (RGBA)" << endl;
		return;
	}

	if (background.channels() != 3) {
		cerr << "Фоновое изображение должно иметь 3 канала (RGB)" << endl;
		return;
	}

	background.copyTo(output);
	Mat resizedTshirt;
	Size targetSize(tshirtSize.width, tshirtSize.height); // Устанавливаем размер футболки
	resize(foreground, resizedTshirt, targetSize);

	// Позиция футболки на фоне
	for (int y = max(location.y, 0); y < background.rows; ++y) {
		int fY = y - location.y;
		if (fY >= resizedTshirt.rows) break;

		for (int x = max(location.x, 0); x < background.cols; ++x) {
			int fX = x - location.x;
			if (fX >= resizedTshirt.cols) break;

			double opacity = ((double)resizedTshirt.at<Vec4b>(fY, fX)[3]) / 255.0;
			for (int c = 0; opacity > 0 && c < output.channels(); ++c) {
				output.at<Vec3b>(y, x)[c] =
					saturate_cast<uchar>(background.at<Vec3b>(y, x)[c] * (1.0 - opacity) +
						resizedTshirt.at<Vec4b>(fY, fX)[c] * opacity);
			}
		}
	}
}

// Функция для извлечения ключевых точек
vector<Point> detectBodyKeypoints(const Mat& person)
{
	vector<Point> keypoints;
	cout << "Загрузка модели нейронной сети..." << endl;

	string modelPath = "H:/OutfitME/outfit_me/clTest/x64/Debug/pose_iter_584000.caffemodel";
	string protoPath = "H:/OutfitME/outfit_me/openpose/models/pose/body_25/pose_deploy.prototxt";

	Net net = readNet(modelPath, protoPath);
	if (net.empty()) {
		cerr << "Ошибка загрузки модели" << endl;
		return keypoints;
	}

	cout << "Подготовка изображения для нейронной сети..." << endl;
	Mat blob;
	blobFromImage(person, blob, 1.0 / 255.0, Size(368, 368), Scalar(0, 0, 0), true, false);

	cout << "Запуск распознавания ключевых точек..." << endl;
	net.setInput(blob);
	Mat output = net.forward();

	const int NUM_KEYPOINTS = 25;
	for (int i = 0; i < NUM_KEYPOINTS; ++i) {
		float x = output.at<float>(0, i, 0) * person.cols;
		float y = output.at<float>(0, i, 1) * person.rows;
		keypoints.push_back(Point(static_cast<int>(x), static_cast<int>(y)));
	}

	cout << "Найдено ключевых точек: " << keypoints.size() << endl;
	return keypoints;
}

int main() {
	setlocale(LC_CTYPE, "Rus");

	// Загрузка изображения человека
	Mat person = imread("H:/OutfitME/outfit_me/clTest/x64/Debug/nikita.jpg");
	if (person.empty()) {
		cerr << "[ERROR] Не удалось загрузить изображение человека!" << endl;
		return -1;
	}
	cout << "[INFO] Размер изображения человека: " << person.size() << endl;

	// Загрузка изображения футболки
	Mat tshirt = imread("H:/OutfitME/outfit_me/assets/images/tshirt.png");
	if (tshirt.empty()) {
		cerr << "[ERROR] Не удалось загрузить изображение футболки!" << endl;
		return -1;
	}
	cout << "[INFO] Размер изображения футболки: " << tshirt.size() << endl;

	// Проверка альфа-канала и преобразование в RGBA
	if (tshirt.channels() == 3) {
		cvtColor(tshirt, tshirt, COLOR_RGB2BGRA);
	}
	// Инициализация нейросети
	Net net = readNetFromCaffe("H:/OutfitME/outfit_me/openpose/models/pose/body_25/pose_deploy.prototxt",
		"H:/OutfitME/outfit_me/clTest/x64/Debug/pose_iter_584000.caffemodel");
	if (net.empty()) {
		cerr << "[ERROR] Не удалось загрузить нейронную сеть!" << endl;
		return -1;
	}
	cout << "[INFO] Модель нейросети успешно загружена." << endl;

	// Подготовка изображения для нейросети
	Mat blob = blobFromImage(person, 1.0 / 255.0, Size(368, 368), Scalar(0, 0, 0), true, false);
	net.setInput(blob);
	Mat output = net.forward();

	// Получаем ключевые точки тела
	vector<Point> keypoints;
	for (int i = 0; i < 25; i++) {
		Mat heatmap = output.row(0).col(i).reshape(1, output.size[2]);
		Point maxLoc;
		double maxVal;
		minMaxLoc(heatmap, nullptr, &maxVal, nullptr, &maxLoc);

		int scaled_x = static_cast<int>((maxLoc.x / static_cast<float>(output.size[3])) * person.cols);
		int scaled_y = static_cast<int>((maxLoc.y / static_cast<float>(output.size[2])) * person.rows);

		keypoints.push_back(Point(scaled_x, scaled_y));
		cout << "Keypoint " << i << ": (" << scaled_x << ", " << scaled_y << ")" << endl;
	}

	// Отображение ключевых точек на изображении
	Mat person_with_keypoints = person.clone();
	for (const auto& keypoint : keypoints) {
		circle(person_with_keypoints, keypoint, 5, Scalar(0, 255, 0), -1);
	}
	imwrite("debug_step1_person_with_keypoints.jpg", person_with_keypoints);
	cout << "[DEBUG] Ключевые точки на изображении сохранены." << endl;

	// Основная точка для наложения футболки - грудная клетка (точка 1)
	Point tshirtLocation(keypoints[1].x - tshirt.cols / 4, keypoints[1].y - tshirt.rows / 2);

	// Рассчитываем масштаб футболки, основываясь на размерах тела
	int bodyWidth = abs(keypoints[5].x - keypoints[2].x);  // расстояние между плечами
	int tshirtWidth = tshirt.cols;
	float scaleFactor = static_cast<float>(bodyWidth) / tshirtWidth;
	// Масштабируем футболку (без деформации)
	Size tshirtSize(static_cast<int>(tshirt.cols * scaleFactor), static_cast<int>(tshirt.rows * scaleFactor));

	// Отображение масштаба футболки
	Mat resized_tshirt;
	resize(tshirt, resized_tshirt, tshirtSize);
	imshow("Resized T-Shirt", resized_tshirt);
	imwrite("debug_step2_resized_tshirt.jpg", resized_tshirt);
	cout << "[DEBUG] Масштабированная футболка сохранена." << endl;

	// Наложение футболки на изображение
	Mat output_with_tshirt;
	overlayImage(person, tshirt, output_with_tshirt, tshirtLocation, tshirtSize);

	// Сохранение изображения с наложенной футболкой
	imwrite("debug_step3_with_tshirt.jpg", output_with_tshirt);
	cout << "[DEBUG] Изображение с наложенной футболкой сохранено." << endl;

	// Отображение финального изображения
	imshow("Final Output with T-Shirt", output_with_tshirt);
	waitKey(0);

	return 0;
}

/*#include <opencv2/opencv.hpp>
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