#pragma once

#include <string>
#include <nlohmann/json.hpp>

class ImageProcessor {
    using json = nlohmann::json;

public:
    ImageProcessor();

    ~ImageProcessor();

    // Function to extract image metadata using libvips
    json getImageInfo(const std::string& filename);

    // Преобразование в RGB с глубиной 24 бита с использованием ImageMagick
    void convertToRGBWithImageMagick(const std::string& filepath);

    // Преобразование в градации серого с использованием VIPS
    void convertToGrayscaleWithVIPS(const std::string& filepath);

    // Преобразование в 8-битное изображение
    void convertTo8bits(const std::string& filepath);

    // Проверка, является ли изображение черно-белым
    bool detectGray(const std::vector<std::string>& pixels);

    // Преобразование в градации серого, если изображение черно-белое
    void convertToGrayIfGray(const std::string& filepath, int reduceColor = 0);

    // Оптимизация JPEG
    void optimizeJpg(const std::string& filepath);

    // Оптимизация PNG
    void optimizePng(const std::string& filepath, bool to8Bits, int reduceColor);

    // Главная функция оптимизации файла
    std::pair<size_t, size_t> optimizeFile(const std::string& filepathOrig, bool to8Bits = false, const std::string& pngMode = "");
};