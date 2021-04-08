#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <ostream>
#include <fstream>

struct block {
    unsigned char Data[8];
};


void operator^(block &Text, const block &Gamma) {
    for (int i = 0; i < 8; i++) {
        Text.Data[i] = Text.Data[i] ^ Gamma.Data[i];
    }
}

void operator>>(block &Block, const int &Number) {
    unsigned char Shift = 0;
    for (int i = 0; i < 8; i++) {
        unsigned char SavedData = Block.Data[i];
        Block.Data[i] >>= Number;
        Block.Data[i] |= Shift;
        Shift = (SavedData ^ (Block.Data[i] << Number)) << (8 - Number);
    }
    Block.Data[0] += Shift;
}

void operator<<(block &Block, const int &Number) {
    unsigned char Shift = 0;
    for (int i = 7; i > -1; i--) {
        unsigned char SavedData = Block.Data[i];
        Block.Data[i] <<= Number;
        Block.Data[i] |= Shift;
        Shift = (SavedData ^ (Block.Data[i] >> Number)) >> (8 - Number);
    }
    Block.Data[7] += Shift;
}

void Encrypting(std::string &Text, const std::string &Key, const std::string &FileOut) {
    Text = "  " + Text;
    float A = Text.size() / 8.0;
    float B = static_cast<float>(Text.size() / 8);
    unsigned int Amount = 0;

    if (A - B == 0)
        Amount = static_cast<unsigned int>(B);
    else
        Amount = static_cast<unsigned int>(B) + 1;


    block *TextInBlocks = new block[Amount];
    TextInBlocks[0].Data[0] = Amount;
    TextInBlocks[0].Data[1] = Text.size() - 2;
    std::srand(std::time(nullptr));
    for (int i = 0; i < Amount; i++) {
        if (Text.size() >= 8) {
            std::string PieceText = Text.substr(0, 8);
            for (int j = 0; j < 8; j++) {
                if (!((i == 0) && ((j == 0) || (j == 1))))
                    TextInBlocks[i].Data[j] = PieceText[j];
            }
        } else {
            for (int j = 0; j < Text.size(); j++)
                TextInBlocks[i].Data[j] = Text[j];
            for (int j = Text.size(); j < (8 - Text.size()); j++)
                TextInBlocks[i].Data[j] = rand() % 128;
        }
        if (Text.size() >= 8)
            Text = Text.substr(8, Text.size() - 8);
    }
    //Creating Gamma and xoring Text with Gamma
    long long int IV = stoi(Key);
    std::srand(IV);
    for (int i = 0; i < Amount; i++) {
        block Gamma;
        for (int j = 0; j < 8; j++) {
            Gamma.Data[j] = rand() % 128;
        }
        TextInBlocks[i] ^ Gamma;
    }

    int ShiftNumber = 7;
    for (int i = 0; i < Amount; i++) {
        TextInBlocks[i] >> ShiftNumber;
    }

    std::ofstream outputFile(FileOut, std::ios::binary);
    for (int i = 0; i < Amount; i++)
        for (int j = 0; j < 8; j++)
            outputFile.write(reinterpret_cast<char *>(&TextInBlocks[i].Data[j]), 1);
    outputFile.close();

    delete[] TextInBlocks;
};

void Decrypting(block *TextInBlocks, const std::string &FileOut) {

    for (int i = 1; i < TextInBlocks[0].Data[0]; i++)
        TextInBlocks[i] << 7;

    //xor
    for (int i = 1; i < TextInBlocks[0].Data[0]; i++) {
        block Gamma;
        for (int j = 0; j < 8; j++) {
            Gamma.Data[j] = rand() % 128;
        }
        TextInBlocks[i] ^ Gamma;
    }

    unsigned int Size = TextInBlocks[0].Data[1];
    std::string DecryptedText;
    for (int i = 0; i < TextInBlocks[0].Data[0]; i++) {
        for (int j = 0; j < 8; j++) {
            if (!((i == 0) && ((j == 0) || (j == 1))))
                DecryptedText += TextInBlocks[i].Data[j];
        }
    }
    DecryptedText = DecryptedText.substr(0, Size);

    std::ofstream outputFile(FileOut);
    outputFile << DecryptedText;
    outputFile.close();
    delete [] TextInBlocks;
}

int main(int argc, char *argv[]) {
    std::string Mod = argv[1];
    std::string FileIn = argv[2];
    std::string IV = argv[3];
    std::string FileOut = argv[4];

    if (Mod == "Encrypting") {
        std::ifstream inputFile(FileIn);
        std::string Text;
        std::getline(inputFile, Text, '\0');
        inputFile.close();
        Encrypting(Text, IV, FileOut);
        std::cout << "Text was Encrypted" << std::endl;
    } else if (Mod == "Decrypting") {
        std::ifstream inputFile(FileIn, std::ios::binary);
        block GetAmount;
        for (int j = 0; j < 8; j++)
            inputFile.read(reinterpret_cast<char *>(&GetAmount.Data[j]), 1);
        GetAmount << 7;
        long long int Key = stoi(IV);
        std::srand(Key);
        block FirstGamma;
        for (int j = 0; j < 8; j++)
            FirstGamma.Data[j] = rand() % 128;
        GetAmount ^ FirstGamma;
        unsigned char Amount = GetAmount.Data[0];
        block *TextInBlocks = new block[Amount];
        TextInBlocks[0] = GetAmount;
        for (int i = 1; i < Amount; i++)
            for (int j = 0; j < 8; j++)
                inputFile.read(reinterpret_cast<char *>(&TextInBlocks[i].Data[j]), 1);
        Decrypting(TextInBlocks, FileOut);
        std::cout << "Text was decrypted" << std::endl;
    }
    return 0;
}
