#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

class SaveFile {
public:
    SaveFile() = default;
    SaveFile(const std::string nFileName) {
        sFileName = nFileName;
        mInputFile.open(nFileName, std::ios::in | std::ios::app);
        std::stringstream ss;
        ss << mInputFile.rdbuf();
        sContent = ss.str();
        ss.clear();
        mOutputFile.open(nFileName, std::ios::app | std::ios::out);
    }
    void UpdateFile() {
        if (mOutputFile.is_open()) mOutputFile.close();
        mOutputFile.open(sFileName);
        mOutputFile << sContent;
        mOutputFile.close();
        mInputFile.close();
    }
    void Write(const std::string id, std::size_t nLocation = 0) {
        std::string str;
        if (nLocation >= sContent.size()) {
            sContent.append(id);
            return;
        }
        for (std::size_t i = 0; i < sContent.size(); ++i) {
            if (i < nLocation) {
                str += sContent[i];
            }
            else if (i == nLocation) {
                for (int j = 0; j < id.size(); j++) {
                    str += id[j];
                }
            }
            else if(i > nLocation) {
                if (i == nLocation + 1)
                    str += sContent[i - 1];
                str += sContent[i];
            }
        }
        sContent.clear();
        sContent.append(str);
    }
    std::size_t GetNewLine(std::size_t nLine) {
        std::size_t count = 0;
        for (int i = 0; i < sContent.size(); ++i) {
            if (sContent[i] == '\n')
                nLine--;
            if (nLine == 1)
                return i+1;
        }
        return 0;
    }
    std::size_t GetLineBreaks() {
        std::size_t nCount = 0;
        for (int i = 0; i < sContent.size(); i++)
            if (sContent[i] == '\n')
                nCount++;
        return nCount;
    }
    void OverWrite(std::string id, std::size_t nLocation = 0) {
        std::string str;
        uint32_t nSize = (nLocation + id.size() > sContent.size()) ? (nLocation + id.size()) : sContent.size();
        for (int i = 0; i < nSize; ++i) {
            if (i < nLocation)
                str += sContent[i];
            else if (i >= nLocation && i < nLocation + id.size())
                str += id[i - nLocation];
            else
                str += sContent[i];
        }
        sContent.clear();
        sContent.append(str);
    }
    void Clear() {
        sContent.clear();
    }
    std::string Read(const std::size_t nSize, std::size_t nLocation = 0) {
        if (nLocation > sContent.size())
            nLocation = sContent.size();
        std::string str;
        for (std::size_t i = 0; i < sContent.size(); ++i)
            if (i >= nLocation && i < nLocation + nSize) 
                str += sContent[i];
        return str;
    }
    std::string ReadBetween(const std::size_t sPoint, const std::size_t ePoint) {
        return Read(ePoint - sPoint, sPoint);
    }
    void DeleteBetween(const std::size_t nStart, const std::size_t nEnd) {
        Delete(nStart, nEnd - nStart);
    }
    std::size_t FindEnd(std::string id) {
        return Find(id) + id.size();
    }
    void Delete(std::size_t nLocation, const std::size_t nSize) {
        if (nLocation + nSize > sContent.size())
            nLocation = sContent.size() - nSize;
        std::string str;
        for (std::size_t i = 0; i < sContent.size(); ++i) {
            if (i >= nLocation && i < nSize + nLocation)
                continue;
            else
                str += sContent[i];
        }
        sContent.clear();
        sContent.append(str);
    }
    std::size_t Find(const std::string& id) {
        for (std::size_t i = 0; i < sContent.size(); ++i) 
            if (strcmp(sContent.substr(i, id.size()).c_str(), id.c_str()) == 0) 
                return i;
        return std::string::npos;
    }
    void Append(std::string data) {
        sContent.append(data);
    }
    bool isEmpty() {
        return sContent.empty();
    }
    std::string GetContent() {
        return sContent;
    }
private:
    std::string sContent;
    std::ofstream mOutputFile;
    std::ifstream mInputFile;
    std::string sFileName;
};