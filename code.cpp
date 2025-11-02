#include <iostream>
#include <fstream>
#include <string>
#include <iomanip> 

using namespace std;

struct Image {
    int width;
    int height;
    int maxValue;
    int** pixels;
};

int readNextNumber(ifstream& file) {
    string numberStr;
    char ch;
    
    while (file.get(ch)) {
        if (ch >= '0' && ch <= '9') {
            numberStr += ch;
            break;
        }
    }
    
    while (file.get(ch)) {
        if (ch >= '0' && ch <= '9') {
            numberStr += ch;
        } else {
            file.putback(ch); 
            break;
        }
    }
    
    if (numberStr.empty()) return 0;
    return stoi(numberStr);
}

Image readPGM(string filename) {
    Image img;
    ifstream file(filename, ios::binary);
    
    if (!file.is_open()) {
        cout << "ERROR: Cannot open file " << filename << endl;
        img.width = 0;
        img.height = 0;
        return img;
    }
    
    string magic;
    char ch;
    file.get(ch); magic += ch;  
    file.get(ch); magic += ch; 
    
    cout << "File format: " << magic << endl;
    
    if (magic != "P2") {
        cout << "ERROR: This is not a P2 PGM file!" << endl;
        img.width = 0;
        img.height = 0;
        return img;
    }
    
    while (file.get(ch) && ch != '\n') {}
    
    while (file.peek() == '#') {
        while (file.get(ch) && ch != '\n') {}
    }
    
    img.width = readNextNumber(file);
    img.height = readNextNumber(file);
    img.maxValue = readNextNumber(file);
    
    cout << "Size: " << img.width << "x" << img.height << endl;
    cout << "Max value: " << img.maxValue << endl;
    
    if (img.width <= 0 || img.height <= 0) {
        cout << "ERROR: Invalid image dimensions" << endl;
        return img;
    }

    img.pixels = new int*[img.height];
    for (int i = 0; i < img.height; i++) {
        img.pixels[i] = new int[img.width];
        for (int j = 0; j < img.width; j++) {
            img.pixels[i][j] = readNextNumber(file);
        }
        
        if (i % 50 == 0) {
            cout << "Reading... " << i << "/" << img.height << " rows" << endl;
        }
    }
    
    file.close();
    return img;
}

void printSample(Image img, int rows = 5, int cols = 5) {
    cout << "Sample of first " << rows << "x" << cols << " pixels:" << endl;
    for (int i = 0; i < rows && i < img.height; i++) {
        for (int j = 0; j < cols && j < img.width; j++) {
            cout << img.pixels[i][j] << "\t";
        }
        cout << endl;
    }
}

void cleanupImage(Image img) {
    if (img.pixels != nullptr) {
        for (int i = 0; i < img.height; i++) {
            delete[] img.pixels[i];
        }
        delete[] img.pixels;
    }
}

Image copyImage(Image original) {
    Image copy;
    copy.width = original.width;
    copy.height = original.height;
    copy.maxValue = original.maxValue;
    
    copy.pixels = new int*[copy.height];
    for (int i = 0; i < copy.height; i++) {
        copy.pixels[i] = new int[copy.width];
        for (int j = 0; j < copy.width; j++) {
            copy.pixels[i][j] = original.pixels[i][j];
        }
    }
    
    return copy;
}

Image medianFilter(Image original, int windowSize) {
    Image filtered = copyImage(original);

    if (windowSize % 2 == 0) {
        cout << "ERROR: Window size must be odd!" << endl;
        return filtered;
    }
    
    int offset = windowSize / 2; 
    
    cout << "Applying median filter " << windowSize << "x" << windowSize << "..." << endl;

    for (int i = offset; i < original.height - offset; i++) {
        for (int j = offset; j < original.width - offset; j++) {

            int* window = new int[windowSize * windowSize];
            int index = 0;

            for (int wi = -offset; wi <= offset; wi++) {
                for (int wj = -offset; wj <= offset; wj++) {
                    window[index] = original.pixels[i + wi][j + wj];
                    index++;
                }
            }

            for (int k = 0; k < windowSize * windowSize - 1; k++) {
                for (int m = k + 1; m < windowSize * windowSize; m++) {
                    if (window[k] > window[m]) {
                        int temp = window[k];
                        window[k] = window[m];
                        window[m] = temp;
                    }
                }
            }
            
            int median = window[windowSize * windowSize / 2];
            filtered.pixels[i][j] = median;
            
            delete[] window;
        }
        
        if (i % 50 == 0) {
            cout << "Processed " << i << "/" << original.height << " rows" << endl;
        }
    }
    
    cout << "Filtering completed!" << endl;
    return filtered;
}

void savePGM(string filename, Image img) {
    string path = "photo/" + filename; 
    
    ofstream file(path);
    
    if (!file.is_open()) {
        cout << "ERROR: Cannot create file " << path << endl;
        return;
    }

    file << "P2" << endl;
    file << "# Processed image" << endl;
    file << img.width << " " << img.height << endl;
    file << img.maxValue << endl;

    for (int i = 0; i < img.height; i++) {
        for (int j = 0; j < img.width; j++) {
            file << img.pixels[i][j] << " ";
        }
        file << endl;
    }
    
    file.close();
    cout << "Image saved: " << path << endl;
}

double calculateSimilarity(Image img1, Image img2, int tolerance = 5) {
    int similarPixels = 0;
    int totalPixels = 0;

    for (int i = 0; i < img1.height && i < img2.height; i++) {
        for (int j = 0; j < img1.width && j < img2.width; j++) {
            int diff = abs(img1.pixels[i][j] - img2.pixels[i][j]);
            if (diff <= tolerance) {
                similarPixels++;
            }
            totalPixels++;
        }
    }
    
    return (similarPixels * 100.0) / totalPixels;
}

void saveResultsToCSV(string filename, string imageName, string filterName, 
                     int windowSize, double similarity) {
    ofstream csv(filename, ios::app);
    
    if (!csv.is_open()) {
        cout << "ERROR: Cannot create CSV file" << endl;
        return;
    }
    
    csv.seekp(0, ios::end);
    if (csv.tellp() == 0) {
        csv << "Image;Filter;Window_Size;Similarity_%" << endl;
    }
    
    csv << fixed << setprecision(2);
    csv << imageName << ";" << filterName << ";" << windowSize << "x" << windowSize << ";" << similarity << endl;
    
    csv.close();
}

int main() {
    cout << "=== IMAGE NOISE REMOVAL AND COMPARISON PROGRAM ===" << endl;
    
    string filename;
    cout << "Enter PGM filename: ";
    cin >> filename;
    
    string csvFilename = "results.csv";
    
    cout << "\n=== PROCESSING IMAGE: " << filename << " ===" << endl;
    
    Image original = readPGM(filename);
    
    if (original.width == 0) {
        cout << "Failed to load image!" << endl;
        return 1;
    }
    
    cout << "\n=== IMAGE INFORMATION ===" << endl;
    cout << "Filename: " << filename << endl;
    cout << "Size: " << original.width << "x" << original.height << endl;
    cout << "Maximum value: " << original.maxValue << endl;
    cout << "Total pixels: " << original.width * original.height << endl;
    
    cout << "\n=== PIXEL SAMPLE ===" << endl;
    printSample(original);
    
    cout << "\n=== APPLYING FILTERS ===" << endl;
    
    Image filtered3x3 = medianFilter(original, 3);
    Image filtered5x5 = medianFilter(original, 5);
    
    cout << "\n=== COMPARISON RESULTS ===" << endl;
    double similarity3x3 = calculateSimilarity(original, filtered3x3);
    double similarity5x5 = calculateSimilarity(original, filtered5x5);
    
    cout << fixed << setprecision(2);
    cout << "Similarity with original:" << endl;
    cout << " - Median filter 3x3: " << similarity3x3 << "%" << endl;
    cout << " - Median filter 5x5: " << similarity5x5 << "%" << endl;
    
    saveResultsToCSV(csvFilename, filename, "Median", 3, similarity3x3);
    saveResultsToCSV(csvFilename, filename, "Median", 5, similarity5x5);
    
    cout << "\n=== SAVING FILES TO 'photo' FOLDER ===" << endl;
    
    string shortFilename = filename;
    size_t lastSlash = filename.find_last_of("/\\");
    if (lastSlash != string::npos) {
        shortFilename = filename.substr(lastSlash + 1);
    }
    
    savePGM("original_" + shortFilename, original);
    savePGM("filtered_3x3_" + shortFilename, filtered3x3);
    savePGM("filtered_5x5_" + shortFilename, filtered5x5);
    
    cleanupImage(original);
    cleanupImage(filtered3x3);
    cleanupImage(filtered5x5);
    
    cout << "\n=== COMPLETED! ===" << endl;
    cout << "Results saved to file: " << csvFilename << endl;
    cout << "Created files in 'photo' folder:" << endl;
    cout << " - original_" << shortFilename << " (original)" << endl;
    cout << " - filtered_3x3_" << shortFilename << " (3x3 filter)" << endl;
    cout << " - filtered_5x5_" << shortFilename << " (5x5 filter)" << endl;
    
    return 0;
}