#include <iostream>
#include <filesystem>

#include "image_processor.h"

using namespace std;
namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <image_file>" << endl;
        return EXIT_FAILURE;
    }

    const string filename = argv[1];
    if (!fs::exists(filename)) {
        cerr << "File not found: " << filename << endl;
        return EXIT_FAILURE;
    }

    // Get image metadata
    const auto imageInfo = ImageProcessor().getImageInfo(filename);

    // Print the metadata as a formatted JSON string
    cout << imageInfo.dump(4) << endl; // Use dump(4) for pretty-printing with indentation

    return EXIT_SUCCESS;
}