#include <opencv2/opencv.hpp>
#include <iostream>
#include <opencv2/dnn.hpp>
#include <string>

using namespace cv;
using namespace std;
using namespace dnn;

// Константы
const string MODEL_PATH = "pose_iter_584000.caffemodel";
const string PROTO_PATH = "pose_deploy.prototxt";
const Size BLOB_SIZE(350, 350);
const int NUM_KEYPOINTS = 25;  // 0-24

// Определение структуры BodyKeypoints
struct BodyKeypoints {
    struct Head {
        Point nose;        // 0
        Point neck;        // 1
        Point rightEye;    // 14
        Point leftEye;     // 15
        Point rightEar;    // 16
        Point leftEar;     // 17
    };

    struct Torso {
        Point rightShoulder;  // 2
        Point leftShoulder;   // 5
        Point rightHip;       // 8
        Point leftHip;        // 11
    };

    struct Arms {
        Point rightElbow;     // 3
        Point leftElbow;      // 6
        Point rightWrist;     // 4
        Point leftWrist;      // 7
    };

    struct Legs {
        Point rightKnee;      // 9
        Point leftKnee;       // 12
        Point rightAnkle;     // 10
        Point leftAnkle;      // 13
    };

    struct Feet {
        Point rightToe;      // 18
        Point leftToe;       // 19
        Point rightHeel;     // 20
        Point leftHeel;      // 21
        Point rightBall;     // 22
        Point leftBall;      // 23
        Point rightMid;      // 24
    };

    Head head;
    Torso torso;
    Arms arms;
    Legs legs;
    Feet feet;

    static BodyKeypoints fromVector(const vector<Point>& keypoints) {
        BodyKeypoints body;

        if (keypoints.size() < NUM_KEYPOINTS) {
            throw runtime_error("Недостаточно ключевых точек в векторе");
        }

        // Заполнение головы
        body.head.nose = keypoints[0];
        body.head.neck = keypoints[1];
        body.head.rightEye = keypoints[14];
        body.head.leftEye = keypoints[15];
        body.head.rightEar = keypoints[16];
        body.head.leftEar = keypoints[17];

        // Заполнение торса
        body.torso.rightShoulder = keypoints[2];
        body.torso.leftShoulder = keypoints[5];
        body.torso.rightHip = keypoints[8];
        body.torso.leftHip = keypoints[11];

        // Заполнение рук
        body.arms.rightElbow = keypoints[3];
        body.arms.leftElbow = keypoints[6];
        body.arms.rightWrist = keypoints[4];
        body.arms.leftWrist = keypoints[7];

        // Заполнение ног
        body.legs.rightKnee = keypoints[9];
        body.legs.leftKnee = keypoints[12];
        body.legs.rightAnkle = keypoints[10];
        body.legs.leftAnkle = keypoints[13];

        // Заполнение ступней
        body.feet.rightToe = keypoints[18];
        body.feet.leftToe = keypoints[19];
        body.feet.rightHeel = keypoints[20];
        body.feet.leftHeel = keypoints[21];
        body.feet.rightBall = keypoints[22];
        body.feet.leftBall = keypoints[23];
        body.feet.rightMid = keypoints[24];

        return body;
    }

    void drawKeypoints(Mat& image, const Scalar& color = Scalar(0, 0, 255), int radius = 5) {
        // Рисуем точки головы
        circle(image, head.nose, radius, color, -1);
        circle(image, head.neck, radius, color, -1);
        circle(image, head.rightEye, radius, color, -1);
        circle(image, head.leftEye, radius, color, -1);
        circle(image, head.rightEar, radius, color, -1);
        circle(image, head.leftEar, radius, color, -1);

        // Рисуем точки торса
        circle(image, torso.rightShoulder, radius, color, -1);
        circle(image, torso.leftShoulder, radius, color, -1);
        circle(image, torso.rightHip, radius, color, -1);
        circle(image, torso.leftHip, radius, color, -1);

        // Рисуем точки рук
        circle(image, arms.rightElbow, radius, color, -1);
        circle(image, arms.leftElbow, radius, color, -1);
        circle(image, arms.rightWrist, radius, color, -1);
        circle(image, arms.leftWrist, radius, color, -1);

        // Рисуем точки ног
        circle(image, legs.rightKnee, radius, color, -1);
        circle(image, legs.leftKnee, radius, color, -1);
        circle(image, legs.rightAnkle, radius, color, -1);
        circle(image, legs.leftAnkle, radius, color, -1);

        // Рисуем точки ступней
        circle(image, feet.rightToe, radius, color, -1);
        circle(image, feet.leftToe, radius, color, -1);
        circle(image, feet.rightHeel, radius, color, -1);
        circle(image, feet.leftHeel, radius, color, -1);
        circle(image, feet.rightBall, radius, color, -1);
        circle(image, feet.leftBall, radius, color, -1);
        circle(image, feet.rightMid, radius, color, -1);
    }

    void drawConnections(Mat& image, const Scalar& color = Scalar(0, 255, 0), int thickness = 2) {
        // Соединения головы
        line(image, head.nose, head.neck, color, thickness);
        line(image, head.rightEye, head.rightEar, color, thickness);
        line(image, head.leftEye, head.leftEar, color, thickness);

        // Соединения торса
        line(image, torso.rightShoulder, torso.leftShoulder, color, thickness);
        line(image, torso.rightHip, torso.leftHip, color, thickness);
        line(image, torso.rightShoulder, torso.rightHip, color, thickness);
        line(image, torso.leftShoulder, torso.leftHip, color, thickness);

        // Соединения рук
        line(image, torso.rightShoulder, arms.rightElbow, color, thickness);
        line(image, arms.rightElbow, arms.rightWrist, color, thickness);
        line(image, torso.leftShoulder, arms.leftElbow, color, thickness);
        line(image, arms.leftElbow, arms.leftWrist, color, thickness);

        // Соединения ног
        line(image, torso.rightHip, legs.rightKnee, color, thickness);
        line(image, legs.rightKnee, legs.rightAnkle, color, thickness);
        line(image, torso.leftHip, legs.leftKnee, color, thickness);
        line(image, legs.leftKnee, legs.leftAnkle, color, thickness);

        // Соединения ступней
        line(image, legs.rightAnkle, feet.rightHeel, color, thickness);
        line(image, legs.leftAnkle, feet.leftHeel, color, thickness);
        line(image, feet.rightHeel, feet.rightToe, color, thickness);
        line(image, feet.leftHeel, feet.leftToe, color, thickness);
    }
};

// Функция наложения изображения
void overlayImage(const Mat& background, const Mat& foreground, Mat& output, Point2i location) {
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
    for (int y = max(location.y, 0); y < background.rows; ++y) {
        int fY = y - location.y;
        if (fY >= foreground.rows) break;

        for (int x = max(location.x, 0); x < background.cols; ++x) {
            int fX = x - location.x;
            if (fX >= foreground.cols) break;

            double opacity = ((double)foreground.at<Vec4b>(fY, fX)[3]) / 255.;
            for (int c = 0; opacity > 0 && c < output.channels(); ++c) {
                output.at<Vec3b>(y, x)[c] =
                    saturate_cast<uchar>(background.at<Vec3b>(y, x)[c] * (1. - opacity) +
                        foreground.at<Vec4b>(fY, fX)[c] * opacity);
            }
        }
    }
}

void debugSaveImage(const Mat& image, const string& filename) {
    imwrite(filename, image);
    cout << "[DEBUG] Изображение сохранено: " << filename << endl;
}

int main() {
    setlocale(LC_CTYPE, "Rus");

    // Загрузка изображения человека
    Mat person = imread("person.jpg");
    if (person.empty()) {
        cerr << "[ERROR] Не удалось загрузить изображение человека!" << endl;
        return -1;
    }
    cout << "[INFO] Размер изображения человека: " << person.size() << endl;
    debugSaveImage(person, "debug_step1_person.jpg");

    // Загрузка изображения футболки
    Mat tshirt = imread("tshirt.png", IMREAD_UNCHANGED);
    if (tshirt.empty()) {
        cerr << "[ERROR] Не удалось загрузить изображение футболки!" << endl;
        return -1;
    }
    cout << "[INFO] Размер изображения футболки: " << tshirt.size() << endl;

    // Проверяем, есть ли альфа-канал у изображения футболки
    if (tshirt.channels() == 3) {
        cvtColor(tshirt, tshirt, COLOR_RGB2BGRA);
    }
    debugSaveImage(tshirt, "debug_step2_tshirt.png");

    // Инициализация нейросети
    Net net;
    try {
        net = readNetFromCaffe(PROTO_PATH, MODEL_PATH);
        if (net.empty()) {
            throw runtime_error("Ошибка загрузки модели");
        }
    }
    catch (const cv::Exception& e) {
        cerr << "[ERROR] OpenCV exception: " << e.what() << endl;
        return -1;
    }
    catch (const exception& e) {
        cerr << "[ERROR] Exception: " << e.what() << endl;
        return -1;
    }

    cout << "[INFO] Модель нейросети успешно загружена." << endl;

    // Подготовка изображения для нейросети
    Mat blob;
    try {
        blob = blobFromImage(person, 1.0 / 255.0, BLOB_SIZE, Scalar(0, 0, 0), true, false);
        debugSaveImage(blob, "debug_step3_blob.jpg");

        net.setInput(blob);
        Mat output = net.forward();

        // Нормализация данных
        normalize(output, output, 0, 255, NORM_MINMAX);
        output.convertTo(output, CV_8U);
        imwrite("debug_step3_blob_fixed.jpg", output);

        // Получаем размеры изображения и тензора
        int img_width = person.cols;
        int img_height = person.rows;
        int heatmap_width = output.size[3];
        int heatmap_height = output.size[2];

        vector<Point> keypoints;
        for (int i = 0; i < NUM_KEYPOINTS; i++) {
            Mat heatmap = output.row(0).col(i).reshape(1, heatmap_height);

            Point maxLoc;
            double maxVal;
            minMaxLoc(heatmap, nullptr, &maxVal, nullptr, &maxLoc);

            int scaled_x = static_cast<int>((maxLoc.x / static_cast<float>(heatmap_width)) * img_width);
            int scaled_y = static_cast<int>((maxLoc.y / static_cast<float>(heatmap_height)) * img_height);

            keypoints.push_back(Point(scaled_x, scaled_y));
            cout << "[DEBUG] Keypoint " << i << ": (" << scaled_x << ", " << scaled_y
                << "), Confidence: " << maxVal << endl;
        }

        // Создаем структуру BodyKeypoints
        BodyKeypoints body = BodyKeypoints::fromVector(keypoints);

        // Визуализация точек и соединений
        Mat visualization = person.clone();
        body.drawKeypoints(visualization);
        body.drawConnections(visualization);
        debugSaveImage(visualization, "debug_step4_keypoints.jpg");

        // Наложение футболки
        Point2i tshirtLocation(body.head.neck.x - tshirt.cols / 2,
            body.head.neck.y - tshirt.rows / 2);
        Mat output_with_tshirt;
        overlayImage(visualization, tshirt, output_with_tshirt, tshirtLocation);

        // Сохранение финального результата
        imwrite("debug_step5_with_tshirt.png", output_with_tshirt);
        cout << "[DEBUG] Изображение с наложенной футболкой сохранено" << endl;

        // Показываем результат
        imshow("Result", output_with_tshirt);
        waitKey(0);
    }
    catch (const cv::Exception& e) {
        cerr << "[ERROR] OpenCV exception during processing: " << e.what() << endl;
        return -1;
    }
    catch (const exception& e) {
        cerr << "[ERROR] Exception during processing: " << e.what() << endl;
        return -1;
    }

    return 0;
}
