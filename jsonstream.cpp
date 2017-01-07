#include "jsonstream.h"

//Public

JsonStream::JsonStream() {
	lineNum = 1;
	totalLines = 1;
	currentObjectId = 0;
	currentObjectMemberId = 0;
	currentArrayId = 0;
	currentArrayObjectId = 0;
	currentArrayObjectMemberId = 0;
	currentArrayValueId = 0;
	isValidFile = true;
}

JsonStream::JsonStream(int startingId, bool isDifferentObject) {
	lineNum = 1;
	totalLines = 1;
	if (isDifferentObject) {
		currentObjectId = startingId;
		currentObjectMemberId = 0;
		currentArrayId = 0;
	}
	else {
		currentObjectId = 0;
		currentObjectMemberId = 0;
		currentArrayId = startingId;
	}
	currentArrayObjectId = 0;
	currentArrayObjectMemberId = 0;
	currentArrayValueId = 0;
	isValidFile = true;
}

JsonStream::JsonStream(int startingObjectId, int startingArrayId) {
	lineNum = 1;
	totalLines = 1;
	currentObjectId = startingObjectId;
	currentObjectMemberId = 0;
	currentArrayId = startingArrayId;
	currentArrayObjectId = 0;
	currentArrayObjectMemberId = 0;
	currentArrayValueId = 0;
	isValidFile = true;
}

void JsonStream::open(std::string path) {
	inputStream.open(path);
	jPath = path;
}

void JsonStream::input(JsonObject objects[], JsonArray arrays[]) {
	logger.open("logs/JSON_Store.txt");
	do {
		logger << "Storing line # " << lineNum << "...\n";
		std::cout << "Storing line # " << lineNum << "...\n";
		while (isspace(inputStream.peek())) {
			inputStream.ignore(1);
		}
		std::getline(inputStream, lines[lineNum - 1]);
		totalLines = lineNum;
		lineNum++;
	} while (inputStream);
	inputStream.close();

	parseStream.str(lines[0]);
	if (parseStream.peek() != '{') {
		logger << "Failed to store \"" << jPath << ".\" This is likely do the file not existing or the file being in the incorrect format (i.e. missing '{' in first line).\n";
		std::cout << "Failed to store \"" << jPath << ".\" This is likely do the file not existing or the file being in the incorrect format (i.e. missing '{' in first line).\n";
		isValidFile = false;
	}
	logger.close();

	bool isArray = false;
	bool isEmbeddedArray = false;
	bool isObject = false;

	JsonObject tempObject;

	logger.open("logs/JSON_Parse.txt");
	if (isValidFile) {
		for (int i = 1; i < totalLines; i++) {
			parseStream.str(lines[i]);

			if (NameCheck(lines[i])) {
				parseStream.get();
				std::getline(parseStream, currentName, '\"');

				logger << "Found item name: " << currentName << ".\n";
				std::cout << "Found item name: " << currentName << ".\n";

				parseStream.get();
				parseStream.get();
				parseStream.get();

				logger << "Attempting to parse item with name \"" << currentName << ".\" This is on line " << i+1 << ".\n";
				std::cout << "Attempting to parse item with name \"" << currentName << ".\" This is on line " << i+1 << ".\n";

				if (parseStream.peek() == '\"') {
					if (isArray && isObject) {
						parseStream.get();
						std::getline(parseStream, currentValueS, '\"');

						tempObject.setStringMember(currentArrayObjectMemberId, currentName, currentValueS);
						arrays[currentArrayId].setJsonObject(currentArrayObjectId, tempObject);
						currentArrayObjectMemberId++;
					}
					else if (isArray && !isObject) {
						logger << "Failed to parse line " << i << ". Arrays can not have members, only values.\n";
						std::cout << "Failed to parse line " << i << ". Arrays can not have members, only values.\n";
					}
					else if (isObject && !isArray) {
						parseStream.get();
						std::getline(parseStream, currentValueS, '\"');

						objects[currentObjectId].setStringMember(currentObjectMemberId, currentName, currentValueS);
						currentObjectMemberId++;
					}
					else {
						logger << "Failed to parse line " << i+1 << ". It is in the incorrect format.\n";
						std::cout << "Failed to parse line " << i+1 << ". It is in the incorrect format.\n";
					}
				} //if string
				else if (parseStream.peek() == '{') {
					isObject = true;
				} //if object
				else if (parseStream.peek() == '[') {
					if (isArray) {
						isEmbeddedArray = true;
						tempArrayId = currentArrayId;
						tempArrayValueId = currentArrayValueId;
						tempArrayObjectId = currentArrayObjectId;
						tempArrayObjectMemberId = currentArrayObjectMemberId;
					}
					else {
						isArray = true;
					}
				} //if array
				else if (isdigit(parseStream.peek())) {
					if (isArray && isObject) {
						do {
							currentInt += parseStream.get();
						} while (isdigit(parseStream.peek()));

						currentValueI = atoi(currentInt.c_str());

						tempObject.setIntMember(currentArrayObjectMemberId, currentName, currentValueI);
						arrays[currentArrayId].setJsonObject(currentArrayObjectId, tempObject);
						currentArrayObjectMemberId++;
					}
					else if (isArray && !isObject) {
						logger << "Failed to parse line " << i+1 << ". Arrays can not have members, only values.\n";
						std::cout << "Failed to parse line " << i+1 << ". Arrays can not have members, only values.\n";
					}
					else if (isObject && !isArray) {
						do {
							currentInt += parseStream.get();
						} while (isdigit(parseStream.peek()));

						currentValueI = atoi(currentInt.c_str());

						objects[currentObjectId].setIntMember(currentArrayObjectMemberId, currentName, currentValueI);
						currentArrayObjectMemberId++;
					}
					else {
						logger << "Failed to parse line " << i+1 << ". It is in the incorrect format.\n";
						std::cout << "Failed to parse line " << i+1 << ". It is in the incorrect format.\n";
					}
				} //if digit
				else {
					if (isArray) {
						arrays[currentArrayId].setJsonValue(currentArrayValueId, currentName);
						currentArrayValueId++;
					}
					else {
						logger << "Failed to parse line " << i+1 << ". It is in the incorrect format.\n";
						std::cout << "Failed to parse line " << i+1 << ". It is in the incorrect format.\n";
					}
				}
			} //if name
			else if (IntCheck(lines[i])) {
				do {
					currentInt += parseStream.get();
				} while (isdigit(parseStream.peek()));

				if (isArray) {
					arrays[currentArrayId].setJsonValue(currentArrayValueId, atoi(currentInt.c_str()));
					currentArrayValueId++;
				}
				else {
					logger << "Failed to parse line " << i+1 << ". It is in the incorrect format.\n";
					std::cout << "Failed to parse line " << i+1 << ". It is in the incorrect format.\n";
				}
			}
			else {
				if (parseStream.peek() == '}') {
					isObject = false;
					if (isArray) {
						currentArrayObjectId++;
						currentArrayObjectMemberId = 0;
					}
					else {
						currentObjectId++;
						currentObjectMemberId = 0;
					}
				}
				else if (parseStream.peek() == '{') {
					isObject = true;
				}
				else if (parseStream.peek() == ']') {
					if (!isEmbeddedArray) {
						isArray = false;
						currentArrayId++;
						currentArrayObjectId = 0;
						currentArrayObjectMemberId = 0;
						currentArrayValueId = 0;
					}
					else {
						isEmbeddedArray = false;
						currentArrayId = tempArrayId;
						currentArrayValueId = tempArrayValueId;
						currentArrayObjectId = tempArrayObjectId;
						currentArrayObjectMemberId = tempArrayObjectMemberId;
					}
				}
				else if (parseStream.peek() == '[') {
					if (isArray) {
						isEmbeddedArray = true;
					}
					isArray = true;
				}
				else {
					logger << "Failed to parse line " << i+1 << ". Either the line is in the incorrect format or it is blank. If this is the final line in the .json and no other errors have occured, ignore this message.\n";
					std::cout << "Failed to parse line " << i+1 << ". Either the line is in the incorrect format or it is blank. If this is the final line in the .json and no other errors have occured, ignore this message.\n";
				}
			} //else
		} //for
	} //if isValidFile
	else {
		logger << "Failed to parse \"" << jPath << ".\" This is likely do the file not existing or the file being in the incorrect format (i.e. missing '{' in first line).\n";
		std::cout << "Failed to parse \"" << jPath << ".\" This is likely do the file not existing or the file being in the incorrect format (i.e. missing '{' in first line).\n";
	} //else

	logger.close();
} //void

void JsonStream::close() {
	inputStream.close();
}

//Private

bool JsonStream::NameCheck(std::string input) {
	parseStream.str(input);
	if (parseStream.peek() == '\"') {
		return true;
	}else {
		return false;
	}
}

bool JsonStream::IntCheck(std::string input) {
	parseStream.str(input);
	if (isdigit(parseStream.peek())) {
		return true;
	}
	else {
		return false;
	}
}

/*void JsonStream::PrintLine4() {
	std::cout << lines[3];
}*/