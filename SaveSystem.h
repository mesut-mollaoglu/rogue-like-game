#pragma once
#include"StateMachine.h"

class SaveSystem {
public:
	static void WriteData(std::string data, bool lineBreak = false, size_t lineNumber = -1) {
		if (!SaveSystem::currentFile.is_open()) return;
		if (lineNumber != -1) SaveSystem::GotoLine(currentFile, lineNumber);
		if(!lineBreak) SaveSystem::currentFile << data;
		else SaveSystem::currentFile << data << '\n';
	}
	static std::string ReadLine(size_t lineNumber = -1) {
		if (!SaveSystem::currentFile.is_open()) return "";
		std::string str;
		if (lineNumber != -1) SaveSystem::GotoLine(SaveSystem::currentFile, lineNumber);
		std::getline(SaveSystem::currentFile, str);
		return str;
	}
	static std::string ReadData(size_t lineNumber = -1) {
		if (!SaveSystem::currentFile.is_open()) return "";
		std::string str;
		if (lineNumber != -1) SaveSystem::GotoLine(SaveSystem::currentFile, lineNumber);
		SaveSystem::currentFile >> str;
		return str;
	}
	static void GotoLine(std::fstream& file, unsigned int num) {
		file.seekg(std::ios::beg);
		for (int i = 0; i < num - 1; ++i) {
			file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		}
	}
	static void FileInit(std::string name) {
		if(currentFile.is_open()) currentFile.close();
		currentFile.open(name.c_str(), std::ios::in | std::ios::out);
		fileName = name;
	}
	static void Exit() {
		if (currentFile.is_open()) currentFile.close();
	}
	static const char* GetFileName() {
		return SaveSystem::fileName.c_str();
	}
	static void ClearCurrentFile() {
		SaveSystem::currentFile.close();
		SaveSystem::currentFile.open(fileName.c_str(), std::ios::out | std::ios::trunc);
	}
	static void DeleteLine(size_t line = 0) {
		std::stringstream ss;
		for (int i = 1; i < GetNumberOfLines(); i++) {
			if (i == line) continue;
			else ss << ReadLine(i) << '\n';
		}
		ClearCurrentFile();
		WriteData(ss.str());
	}
	static size_t GetNumberOfLines() {
		std::string line;
		size_t count = 0;
		if (currentFile.is_open()) {
			while (!currentFile.eof()) {
				ReadLine();
				count++;
			}
		}
		return count;
	}
	static bool isEmpty()
	{
		return SaveSystem::currentFile.peek() == std::ifstream::traits_type::eof();
	}
	static bool isCurrentFile(std::string file) {
		return strcmp(file.c_str(), GetFileName()) == 0;
	}
	static std::string fileName;
	static std::fstream currentFile;
};