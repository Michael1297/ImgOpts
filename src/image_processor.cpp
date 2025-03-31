#include "image_processor.h"

#include <iostream>
#include <string>
#include <filesystem>
#include <vips/vips8>
#include <Magick++.h>
#include <nlohmann/json.hpp>

#include "md5.h"

using namespace std;
namespace fs = std::filesystem;

static std::string colorToString(const Magick::Color& color) {
    std::ostringstream oss;
    oss << "rgb("
        << color.redQuantum() * 255 / 65535 << ","
        << color.greenQuantum() * 255 / 65535 << ","
        << color.blueQuantum() * 255 / 65535 << ")";
    return oss.str();
}

ImageProcessor::ImageProcessor() {
    // Инициализация VIPS
    if (VIPS_INIT(nullptr)) {
        throw std::runtime_error("Failed to initialize VIPS");
    }
    // Инициализация ImageMagick
    Magick::InitializeMagick(nullptr);
}

ImageProcessor::~ImageProcessor() {
    // Очистка VIPS (один раз в конце программы)
    vips_shutdown();
}

// Function to extract image metadata using libvips
// ReSharper disable once CppMemberFunctionMayBeStatic
ImageProcessor::json ImageProcessor::getImageInfo(const string &filename) {
    try {
        // Open the image file using libvips
        const auto image = vips::VImage::new_from_file(filename.c_str());

        // Create a JSON object to store metadata
        json metadata;

        // Extract metadata
        metadata["width"] = image.width();
        metadata["height"] = image.height();
        metadata["bands"] = image.bands();
        metadata["format"] = vips_enum_nick(VIPS_TYPE_BAND_FORMAT, image.format());
        metadata["interpretation"] = vips_enum_nick(VIPS_TYPE_INTERPRETATION, image.interpretation());
        metadata["xres"] = image.xres();
        metadata["yres"] = image.yres();

        // Determine the image type (based on file extension)
        string type = fs::path(filename).extension().string().substr(1);
        metadata["type"] = type;

        return metadata;
    } catch (const vips::VError &e) {
        cerr << "Error: " << e.what() << endl;
        throw;
    }
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void ImageProcessor::convertToRGBWithImageMagick(const std::string &filepath) {
    try {
        // Загрузка изображения
        Magick::Image image;
        image.read(filepath);

        // Преобразование в RGB с глубиной 24 бита
        image.colorSpace(Magick::sRGBColorspace);
        image.depth(24);

        // Сохранение результата
        image.write(filepath);
    } catch (Magick::Exception &e) {
        std::cerr << "ImageMagick error: " << e.what() << std::endl;
        throw;
    }
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void ImageProcessor::convertToGrayscaleWithVIPS(const std::string &filepath) {
    try {
        // Загрузка изображения
        const vips::VImage image = vips::VImage::new_from_file(filepath.c_str());

        // Преобразование в градации серого
        const vips::VImage grayImage = image.colourspace(VIPS_INTERPRETATION_B_W);

        // Сохранение результата
        grayImage.write_to_file(filepath.c_str());
    } catch (vips::VError &e) {
        std::cerr << "VIPS error: " << e.what() << std::endl;
        throw;
    }
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void ImageProcessor::convertTo8bits(const std::string &filepath) {
    try {
        Magick::Image image;
        image.read(filepath);

        // Проверяем текущее цветовое пространство
        if (image.colorSpace() != Magick::sRGBColorspace && image.colorSpace() != Magick::GRAYColorspace) {
            image.depth(8); // Устанавливаем глубину цвета 8 бит
            image.write(filepath);
        }
    } catch (Magick::Exception &e) {
        std::cerr << "Error converting to 8 bits: " << e.what() << std::endl;
        throw;
    }
}

// ReSharper disable once CppMemberFunctionMayBeStatic
bool ImageProcessor::detectGray(const std::vector<std::string> &pixels) {
    int falsePixels = 0;
    constexpr int deltaSize = 30;

    for (const auto &pixel: pixels) {
        if (pixel.empty()) continue;

        // Пример парсинга строки пикселей (упрощенный)
        std::istringstream iss(pixel);
        int r, g, b;
        if (char c; !(iss >> c >> r >> c >> g >> c >> b)) continue;

        if (r == g && g == b) continue; // Полностью серый пиксель

        if (std::abs(r - g) < deltaSize && std::abs(g - b) < deltaSize) continue;

        falsePixels++;
        if (falsePixels > 5) return false; // Найдено слишком много цветных пикселей
    }

    return true;
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void ImageProcessor::optimizeJpg(const std::string &filepath) {
    try {
        Magick::Image image;
        image.read(filepath);

        // Устанавливаем качество JPEG (например, 85%)
        image.quality(85);

        // Сохраняем оптимизированное изображение
        image.write(filepath);
    } catch (Magick::Exception &e) {
        std::cerr << "Error optimizing JPEG: " << e.what() << std::endl;
        throw;
    }
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void ImageProcessor::convertToGrayIfGray(const std::string& filepath, int reduceColor) {
    try {
        Magick::Image image;
        image.read(filepath);

        // Получаем изображение с уникальными цветами
        Magick::Image uniqueColorsImage = image.uniqueColors();

        // Получаем размеры изображения
        size_t width = uniqueColorsImage.columns();
        size_t height = uniqueColorsImage.rows();

        std::vector<std::string> uniqueColors;

        // Перебираем пиксели уникальных цветов
        for (size_t y = 0; y < height; ++y) {
            for (size_t x = 0; x < width; ++x) {
                Magick::Color color = uniqueColorsImage.pixelColor(x, y);
                if (color.alphaQuantum() != 0) { // Игнорируем прозрачные пиксели
                    uniqueColors.push_back(colorToString(color));
                }
            }
        }

        // Проверяем, является ли изображение черно-белым
        if (detectGray(uniqueColors)) {
            std::cout << "Image is grayscale." << std::endl;

            // Преобразуем в градации серого
            image.quantizeColorSpace(Magick::GRAYColorspace);
            image.write(filepath);

            if (reduceColor > 0 && reduceColor < 256) {
                std::cout << "Reducing colors to: " << reduceColor << std::endl;
                image.quantizeColors(reduceColor);
                image.quantize();
                image.write(filepath);
            }
        }
    } catch (Magick::Exception& e) {
        std::cerr << "Error converting to grayscale: " << e.what() << std::endl;
    }
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void ImageProcessor::optimizePng(const std::string &filepath, const bool to8Bits, int reduceColor) {
    try {
        Magick::Image image;
        image.read(filepath);

        if (to8Bits) {
            image.depth(8); // Устанавливаем глубину цвета 8 бит
        }

        // Сжатие PNG
        image.compressType(Magick::ZipCompression);
        image.quality(95); // Уровень качества сжатия

        // Сохраняем оптимизированное изображение
        image.write(filepath);
    } catch (Magick::Exception &e) {
        std::cerr << "Error optimizing PNG: " << e.what() << std::endl;
    }
}

std::pair<size_t, size_t> ImageProcessor::optimizeFile(const std::string &filepathOrig, bool to8Bits,
                                                       const std::string &pngMode) {
    std::cout << "Processing: " << filepathOrig << std::endl;

    Magick::Image image;
    if (!getImageInfo(filepathOrig)) {
        std::cerr << "Failed to get image info: " << filepathOrig << std::endl;
        return {0, 0}; // Возвращаем пустую пару, если произошла ошибка
    }

    std::string ext = fs::path(filepathOrig).extension().string();
    std::string type = ext.substr(1); // Убираем точку

    std::string filepath = "tmp_workfile" + crypto::md5(filepathOrig) + "." + type;

    try {
        // Копируем файл во временную директорию
        fs::copy_file(filepathOrig, filepath, fs::copy_options::overwrite_existing);

        // Сохраняем размеры до оптимизации
        auto sizeBefore = fs::file_size(filepath);
        auto sizeAfter = sizeBefore;

        if (type == "jpg" || type == "jpeg") {
            optimizeJpg(filepath);
        } else if (type == "png") {
            optimizePng(filepath, to8Bits, 0);
        }

        // Обновляем размеры после оптимизации
        sizeAfter = fs::file_size(filepath);

        double percent = (100.0 - static_cast<double>(sizeAfter) * 100.0 / static_cast<double>(sizeBefore));
        double sizeBeforeMB = static_cast<double>(sizeBefore) / 1024.0 / 1024.0;
        double sizeAfterMB = static_cast<double>(sizeAfter) / 1024.0 / 1024.0;
        double sizeSaveMB = (sizeBeforeMB - sizeAfterMB);

        std::cout << "Size before: " << std::fixed << std::setprecision(2) << sizeBeforeMB
                << "MB, after: " << sizeAfterMB << "MB. -" << percent << "% (" << sizeSaveMB << "MB)" << std::endl;

        if (sizeBefore <= sizeAfter) {
            fs::remove(filepath);
        } else {
            std::string newSameDriveFilePath = filepathOrig + crypto::md5(filepathOrig) + "." + type;
            fs::rename(filepath, newSameDriveFilePath);
            fs::rename(newSameDriveFilePath, filepathOrig);
        }

        return {sizeBefore, sizeAfter};
    } catch (const std::exception &e) {
        std::cerr << "Error processing file: " << filepathOrig << ". Error: " << e.what() << std::endl;
        return {0, 0};
    }
}
