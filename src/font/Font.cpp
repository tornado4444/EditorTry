#include "../font/Font.hpp"
#include "../src/Logger/Logger.hpp"
#include "Shader.hpp"

#include <iostream>
#include <fstream>

#pragma warning(once : 6397)

using std::ifstream; using std::string; using std::cout;

Font::Font(const char* const drawing) {
	myFilename = drawing;
	loadFile(drawing);
	loadTextures();
	genVertex();
	MyglobalLogger().logMessage(Logger::DEBUG, "Initialize the font for text.", __FILE__, __LINE__);
}

Font::~Font() {
	for (int i = 0; i < Pages.size(); i++) {
		glDeleteTextures(1, &Pages[i].textId);
		Pages[i].textId = 0;
	}

	if (VAO != 0) {
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
	}
}

int Font::getsSubString(const std::string* const a_Input, std::string* a_Output, int a_Start, char a_Prefix, char a_Suffix) {
	int start = a_Input->find_first_of(a_Prefix, a_Start) + 1;
	int end = a_Input->find_first_of(a_Suffix, start);
	a_Output->assign(a_Input->substr(start, end - start));

	return end;
}

void Font::loadFile(const char* const a_FileName) {
	string line;
	ifstream read = ifstream(a_FileName);

	bool complete = true;
	signed ourLine = 0;
	signed stage = 0;

	signed currentLeft = 0;

	if (read.is_open()) {
		while (std::getline(read, line)) {
			ourLine++;

			string temp;
			size_t end = 0;

			if (stage == 0) {
				//face
				end = getsSubString(&line, &temp, end, '"', '"');
				myInfo.face = temp;

				//size
				end = getsSubString(&line, &temp, end);
				myInfo.size = stoi(temp, nullptr, 10);

				//bold
				end = getsSubString(&line, &temp, end);
				myInfo.bold = stoi(temp, nullptr, 10) ? true : false;

				//char set
				//unused
				end = getsSubString(&line, &temp, end, '"', '"');

				//unicode
				//unused
				end = getsSubString(&line, &temp, end);

				//stretch
				end = getsSubString(&line, &temp, end);
				myInfo.stretchH = stoi(temp, nullptr, 10);

				//smooth
				end = getsSubString(&line, &temp, end);
				myInfo.smooth = stoi(temp, nullptr, 10);

				//aa
				end = getsSubString(&line, &temp, end);
				myInfo.aa = stoi(temp, nullptr, 10);

				//padding
				end = getsSubString(&line, &temp, end, '=', ',');
				myInfo.padding[0] = stoi(temp, nullptr, 10);
				end = getsSubString(&line, &temp, end, ',', ',');
				myInfo.padding[1] = stoi(temp, nullptr, 10);
				end = getsSubString(&line, &temp, end, ',', ',');
				myInfo.padding[2] = stoi(temp, nullptr, 10);
				end = getsSubString(&line, &temp, end, ',', ' ');
				myInfo.padding[3] = stoi(temp, nullptr, 10);

				//spacing
				end = getsSubString(&line, &temp, end, '=', ',');
				myInfo.spacing[0] = stoi(temp, nullptr, 10);
				end = getsSubString(&line, &temp, end, ',', ' ');
				myInfo.spacing[1] = stoi(temp, nullptr, 10);

				//outline
				int start = line.find_last_of('=', line.length()) + 1;
				temp = line.substr(start, end - line.length());
				myInfo.outline = stoi(temp, nullptr, 10);

				stage++;
			}
			else if (stage == 1) {//common
				//line height
				end = getsSubString(&line, &temp, end);
				myPage.lineHeight = stoi(temp, nullptr, 10);

				//base
				end = getsSubString(&line, &temp, end);
				myPage.base = stoi(temp, nullptr, 10);

				//scaleW
				end = getsSubString(&line, &temp, end);
				myPage.scaleW = stoi(temp, nullptr, 10);

				//scaleH
				end = getsSubString(&line, &temp, end);
				myPage.scaleH = stoi(temp, nullptr, 10);

				//pages
				end = getsSubString(&line, &temp, end);
				currentLeft = stoi(temp, nullptr, 10);
				myPage.pages = currentLeft;

				//packed
				end = getsSubString(&line, &temp, end);
				myPage.packed = stoi(temp, nullptr, 10);

				//color channels
				//a
				end = getsSubString(&line, &temp, end);
				myPage.alphaChannel = stoi(temp, nullptr, 10);
				//r
				end = getsSubString(&line, &temp, end);
				myPage.redChannel = stoi(temp, nullptr, 10);
				//g
				end = getsSubString(&line, &temp, end);
				myPage.greenChannel = stoi(temp, nullptr, 10);
				//b
				int start = line.find_last_of('=', line.length()) + 1;
				temp = line.substr(start, end - line.length());
				myPage.blueChannel = stoi(temp, nullptr, 10);

				stage++;

			}
			else if (stage == 2) {//pages
				Page page;

				//id
				end = getsSubString(&line, &temp, end);
				page.id = stoi(temp, nullptr, 10);

				//file name/texture name
				end = getsSubString(&line, &temp, end, '"', '"');
				page.filename = temp;

				//add to vector
				Pages.push_back(page);

				//one less of this stage to do
				currentLeft--;
				if (currentLeft <= 0) {
					stage++;
				}

			}
			else if (stage == 3) {//char amount
				int start = line.find_last_of('=', line.length()) + 1;
				temp = line.substr(start, end - line.length());
				currentLeft = stoi(temp, nullptr, 10);
				
				stage++;
			}
			else if (stage == 4) {//char area
				Character letter;

				//id
				end = getsSubString(&line, &temp, end);
				letter.index = stoi(temp, nullptr, 10);

				//x
				end = getsSubString(&line, &temp, end);
				letter.xCoord = stoi(temp, nullptr, 10);

				//y
				end = getsSubString(&line, &temp, end);
				letter.yCoord = stoi(temp, nullptr, 10);

				//width
				end = getsSubString(&line, &temp, end);
				letter.width = stoi(temp, nullptr, 10);

				//height
				end = getsSubString(&line, &temp, end);
				letter.height = stoi(temp, nullptr, 10);

				//xOffset
				end = getsSubString(&line, &temp, end);
				letter.xOffset = stoi(temp, nullptr, 10);

				//yOffset
				end = getsSubString(&line, &temp, end);
				letter.yOffset = stoi(temp, nullptr, 10);

				//xAdvance
				end = getsSubString(&line, &temp, end);
				letter.xAdvance = stoi(temp, nullptr, 10);

				//page
				end = getsSubString(&line, &temp, end);
				letter.page = stoi(temp, nullptr, 10);

				//channel
				int start = line.find_last_of('=', line.length()) + 1;
				temp = line.substr(start, end - line.length());
				letter.channel = stoi(temp, nullptr, 10);


				//add to map
				Chars[letter.index] = letter;

				//one less of this stage to do
				currentLeft--;
				if (currentLeft <= 0) {
					stage++;
				}
			}
			else if (stage == 5) {//kerning amount
				int start = line.find_last_of('=', line.length()) + 1;
				temp = line.substr(start, end - line.length());
				currentLeft = stoi(temp, nullptr, 10);

				stage++;
			}
			else if (stage == 6) {//kerning area
			int first, second, amount;

			//left character
			end = getsSubString(&line, &temp, end);
			first = stoi(temp, nullptr, 10);

			//right character
			end = getsSubString(&line, &temp, end);
			second = stoi(temp, nullptr, 10);

			//amount
			int start = line.find_last_of('=', line.length()) + 1;
			temp = line.substr(start, end - line.length());
			amount = stoi(temp, nullptr, 10);


			//add to map
			Chars.at(first).kerning[second] = amount;

			//one less of this stage to do
			currentLeft--;
			if (currentLeft <= 0) {
				stage++;
			}
		}
		}

		read.close();
	}
	else {
		complete = false;
	}

	if (!complete) {
		MyglobalLogger().logMessage(Logger::ERROR, "Failed reading font: " + std::string(a_FileName) + " stuck at line: " + std::to_string(ourLine), __FILE__, __LINE__);
	}
}

void Font::loadTextures() {
	string baseFolder = myFilename.substr(0, myFilename.find_last_of('/', myFilename.length()) + 1);

	for (int i = 0; i < Pages.size(); i++) {
		string filename = baseFolder + Pages[i].filename;

		GLenum err = glGetError();
		glGenTextures(1, &Pages[i].textId);
		if (err != GL_NO_ERROR) {
			MyglobalLogger().logMessage(Logger::ERROR, "OpenGL error after glGenTextures: " + std::to_string(err), __FILE__, __LINE__);
		}

		glBindTexture(GL_TEXTURE_2D, Pages[i].textId);

		if (err != GL_NO_ERROR) {
			MyglobalLogger().logMessage(Logger::ERROR, "OpenGL error after glGenTextures: " + std::to_string(err), __FILE__, __LINE__);
		}

		// Parametrs images for rendering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		int width, height, channels;
		unsigned char* image = stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha);

		if (!image) {
			MyglobalLogger().logMessage(Logger::ERROR, "Failed to load texture: " + filename, __FILE__, __LINE__);
			continue;
		}

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		stbi_image_free(image);
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	MyglobalLogger().logMessage(Logger::DEBUG, "Loading font texture from: " + myFilename, __FILE__, __LINE__);
}

void Font::genVertex() {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Font::print(const char* const a_Letters, int const a_X, int const a_Y, float const a_Scale, glm::vec3 a_Color) {
	if (!VAO || !VBO) {
		MyglobalLogger().logMessage(Logger::ERROR, "ERROR TO INITIALIZE TO VAO AND VBO!", __FILE__, __LINE__);
		return;
	}

	// Получаем текущую программу
	GLint currentProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);

	// Получаем локации униформ напрямую
	GLint colorLoc = glGetUniformLocation(currentProgram, "Color");
	GLint imageLoc = glGetUniformLocation(currentProgram, "image");

	if (colorLoc == -1 || imageLoc == -1) {
		MyglobalLogger().logMessage(Logger::ERROR, "Unable to find uniforms in shader!", __FILE__, __LINE__);
		return;
	}

	// Устанавливаем униформы напрямую через OpenGL
	glUniform3f(colorLoc, a_Color.x, a_Color.y, a_Color.z);
	glUniform1i(imageLoc, 0);

	glBindVertexArray(this->VAO);

	float imgHeight = myPage.scaleH;
	float imgWidth = myPage.scaleW;

	float xPos = a_X;
	float yPos = a_Y;

	int index = 0;

	while (a_Letters[index] != '\0') {
		auto iteratorLetter = Chars.find(a_Letters[index]);
		if (iteratorLetter != Chars.end()) {
			Character letter = iteratorLetter->second;

			float sizeWidth = letter.width * a_Scale;
			float sizeHeight = letter.height * a_Scale;

			float uvStartX = letter.xCoord / imgWidth;
			float uvStartY = letter.yCoord / imgHeight;

			float uvWidth = letter.width / imgWidth;
			float uvHeight = letter.height / imgHeight;

			float xOffset = letter.xOffset * a_Scale;
			float yOffset = letter.yOffset * a_Scale;

			GLfloat verts[6][4] = {
				{ xPos + xOffset + 0, yPos + yOffset + 0, uvStartX + 0.0f, uvStartY + 0.0f },
				{ xPos + xOffset + sizeWidth, yPos + yOffset + sizeHeight, uvStartX + uvWidth, uvStartY + uvHeight },
				{ xPos + xOffset + sizeWidth, yPos + yOffset + 0, uvStartX + uvWidth, uvStartY + 0.0f },
				{ xPos + xOffset + 0, yPos + yOffset + 0, uvStartX + 0.0f, uvStartY + 0.0f },
				{ xPos + xOffset + 0, yPos + yOffset + sizeHeight, uvStartX + 0.0f, uvStartY + uvHeight },
				{ xPos + xOffset + sizeWidth, yPos + yOffset + sizeHeight, uvStartX + uvWidth, uvStartY + uvHeight }
			};

			glBindTexture(GL_TEXTURE_2D, Pages[letter.page].textId);

			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			xPos += letter.xAdvance * a_Scale;

			if (a_Letters[index + 1] != '\0') {
				auto num = letter.kerning.find(a_Letters[index + 1]);
				if (num != letter.kerning.end()) {
					xPos += num->second * a_Scale;
				}
			}
		}

		if (a_Letters[index] == '\n') {
			yPos += myPage.lineHeight * a_Scale;
			xPos = a_X;
		}

		index++;
	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

}