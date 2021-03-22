#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <ostream>
#include <fstream>

struct block {
    unsigned char Data[8];
};

struct Encrypt {
    block This;
    Encrypt *next = nullptr;
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
        Block.Data[i] = Block.Data[i] >> Number;
        Block.Data[i] += Shift;
        Shift = (SavedData - (Block.Data[i] << Number)) << (8 - Number);
    }
    Block.Data[0] += Shift;
}

void operator<<(block &Block, const int &Number) {
    unsigned char Shift = 0;
    for (int i = 7; i > -1; i--) {
        unsigned char SavedData = Block.Data[i];
        Block.Data[i] = Block.Data[i] << Number;
        Block.Data[i] += Shift;
        Shift = (SavedData - (Block.Data[i] >> Number)) >> (8 - Number);
    }
    Block.Data[7] += Shift;
}

void Decrypting(std::string &Text, const std::string &Key, const std::string &FileOut) {
    Text=' '+Text;
    float A = Text.size() / 8.0;
    float B = static_cast<float>(Text.size() / 8);
    unsigned int Amount = 0;

    if (A - B == 0)
        Amount = static_cast<unsigned int>(B);
    else
        Amount = static_cast<unsigned int>(B) + 1;


    block *TextInBlocks = new block[Amount];
    TextInBlocks[0].Data[0]=Text.size()-1;
    std::srand(std::time(nullptr));
    for (int i = 0; i < Amount; i++) {
        if (Text.size() >= 8) {
            std::string PieceText = Text.substr(0, 8);
            for (int j = 0; j < 8; j++)
                if ((i != 0) || (j != 0))
                    TextInBlocks[i].Data[j] = PieceText[j];

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
    block *Gamma = new block[Amount];
    for (int i = 0; i < Amount; i++) {
        for (int j = 0; j < 8; j++) {
            Gamma[i].Data[j] = rand() % 128;
        }
    }

    for (int i = 0; i < Amount; i++) {
        TextInBlocks[i] ^ Gamma[i];
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
    delete[] Gamma;
};

void Encrypting(Encrypt &DecryptedText, const std::string &Key, const std::string &FileOut) {

    Encrypt *Current = &DecryptedText;
    unsigned int Amount = 1;
    int ShiftNumber = 7;
    Current->This << ShiftNumber;
    while (Current->next != nullptr) {
        Current = Current->next;
        Current->This << ShiftNumber;
        Amount++;
    }
    //gamma
    long long int IV = stoi(Key);
    std::srand(IV);
    block *Gamma = new block[Amount];
    for (int i = 0; i < Amount; i++) {
        for (int j = 0; j < 8; j++) {
            Gamma[i].Data[j] = rand() % 128;
        }
    }
    //xor
    Current = &DecryptedText;
    for (int i = 0; i < Amount; i++) {
        Current->This ^ Gamma[i];
        Current = Current->next;
    }
    Current = &DecryptedText;
    unsigned int Size = Current->This.Data[0];
    std::string EncryptedText;
    for (int i = 0; i < Amount; i++) {
        for (int j = 0; j < 8; j++) {
            if ((i != 0) || (j != 0))
                EncryptedText += Current->This.Data[j];
        }
        Current = Current->next;
    }
    EncryptedText = EncryptedText.substr(0, Size);
    Current = &DecryptedText;

    std::ofstream outputFile(FileOut);
    outputFile << EncryptedText;
    outputFile.close();

    Encrypt *PreCurrent;
    for (int i = 0; i < Amount; i++) {
        PreCurrent = Current;
        Current = Current->next;
        if (PreCurrent != &DecryptedText)
            delete PreCurrent;

    }
}

int main(int argc, char *argv[]) {
    std::string Mod = argv[1];
    std::string FileIn = argv[2];
    std::string IV = argv[3];
    std::string FileOut = argv[4];

    if (Mod == "Decrypting") {
        std::ifstream inputFile(FileIn);
        std::string Text;
        std::getline(inputFile, Text, '\0');
        inputFile.close();
        Text = Text;
        Decrypting(Text, IV, FileOut);
        std::cout << "Text was decrypted" << std::endl;
    } else if (Mod == "Encrypting") {
        std::ifstream inputFile(FileIn, std::ios::binary);
        Encrypt DecryptedText;
        Encrypt *Current = &DecryptedText;
        unsigned int Size = inputFile.seekg(0, std::ios::end).tellg();
        inputFile.close();
        std::ifstream inputFile2(FileIn, std::ios::binary);
        int count = 0;
        while (count != Size) {
            for (int i = 0; i < 8; i++) {
                unsigned char Letter;
                inputFile2.read(reinterpret_cast<char *>(&Letter), 1);
                Current->This.Data[i] = Letter;
                count++;
            }
            if (count != Size) {
                Current->next = new Encrypt;
                Current = Current->next;
            }
        }
        inputFile2.close();
        Encrypting(DecryptedText, IV, FileOut);
        std::cout << "Text was encrypted" << std::endl;
    }
    return 0;
}
