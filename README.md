# MY OPENGL ENGINE!
Here been my project OpenGL! It's don't my first project, it's just my repeat about OpenGL, and I'm tried to create(how is possibly) greate the engine on OpenGL! The repository is been update every day, why i don't finish this project! Lets start!

## 20.02.25
There is just simple check my triangle! And I'm decided make the font rendering(Signed distance field font rendering), and to be honest, I wanted to do this for a very long time, but I didn’t have enough experience to implement it, so I decided to do it now (for those who don’t know, I will describe everything in detail so that everyone understands exactly what I want to achieve).
### Result.
![gif](https://github.com/tornado4444/OpenGL/blob/main/murder.gif)

How we can see, there are fragment and vertex shaders on my engine. At the moment shaders are implemented for triangle and text(signed text distance field font rendering).

```c++
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

```
Here is the here we specifically enter and set parameters for the background so that everything will be rendered normally.
## 02.03.25
Here is camera and textures. Lets to show you this. 

# TODO
- [X] Signed Distance Field Font Rendering.
- [X] Fixing camera and textures.
- [ ] Guizmo(base for the check).
- [ ] Models.
- [ ] Animation.
- [ ] PBR.
- [ ] AO(Ambient Occlusion).
- [ ] Volumetric Clouds.

# Operating Systems.
- [X] Windows.
- [ ] Linux.
