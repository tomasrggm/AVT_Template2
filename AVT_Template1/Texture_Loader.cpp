#include <GL/glew.h>
#include <GL/freeglut.h>
#include <stdio.h>
#include <IL/il.h>
#include <IL/ilu.h>
//#include <IL/ilut.h>


void Texture2D_Loader(unsigned int *textureArray, const char *strFileName, int ID)
{
	ILuint ImageId;

	//Objecto textura a ser preenchido
	glBindTexture(GL_TEXTURE_2D, textureArray[ID]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	ilGenImages(1, &ImageId);
	ilBindImage(ImageId);
	
	// No OGL o texture 2D mapping usa as duas coordenadas de textura s,t assumindo o lower left como origem
	ilEnable(IL_ORIGIN_SET);
	ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
	if (ilLoadImage(strFileName))
		printf("2D Texture: Image sucessfully loaded.\n");
	else {
		printf("2D Texture: Image could not be loaded.\n");
		exit(0);
	}
	
	ILint bpp = ilGetInteger(IL_IMAGE_BITS_PER_PIXEL);
	
	GLenum format = GL_RGB;
	//printf("bpp=%d\n", bpp);
	
	if (bpp == 24)
		format = GL_RGB;
	else if (bpp == 32)
		format = GL_RGBA;

	ilConvertImage(format, GL_UNSIGNED_BYTE);

	//format=ilGetInteger(IL_IMAGE_FORMAT);
	glTexImage2D(GL_TEXTURE_2D, 0, format, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 0, format, GL_UNSIGNED_BYTE, ilGetData());
	ilDeleteImages(1, &ImageId);
	ilDisable(IL_ORIGIN_SET);
}

void TextureCubeMap_Loader(unsigned int *textureArray, const char **strFileName, int ID)
{
	ILuint ImageName;

	glBindTexture(GL_TEXTURE_CUBE_MAP, textureArray[ID]);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	//Usou-se no init a função glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS) de modo que a escolha abaixo é irrelevante
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
 

	// No OGL, o cube mapping usa as duas coordenadas de textura em cada uma das faces do cubo assumindo o upper left como origem
	ilEnable(IL_ORIGIN_SET);
	ilOriginFunc(IL_ORIGIN_UPPER_LEFT);

	for (int i = 0; i < 6; i++) {
		ilGenImages(1, &ImageName);
		ilBindImage(ImageName);
		 
		if (ilLoadImage(strFileName[i]))  //Image loaded with upper left origin
			printf("Cubemap face %d: Image sucessfully loaded.\n", i);
		else {
			printf("Cubemap face %d: Image could not be loaded.\n", i);
			exit(0);
		}

		ILint bpp = ilGetInteger(IL_IMAGE_BITS_PER_PIXEL);

	/*
		//Apenas para debug 
		ILinfo ImageInfo;
		iluGetImageInfo(&ImageInfo);
		if (ImageInfo.Origin == IL_ORIGIN_UPPER_LEFT)
			printf("Upper left origin Image\n");
		else 
			printf("Lower left origin Image\n");
	*/

		GLenum format = GL_RGB;
		//printf("bpp=%d\n", bpp);
		if (bpp == 24)
			format = GL_RGB;
		else if (bpp == 32)
			format = GL_RGBA;

		ilConvertImage(format, GL_UNSIGNED_BYTE);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 0, format, GL_UNSIGNED_BYTE, ilGetData());
		ilDeleteImages(1, &ImageName);
	}
	ilDisable(IL_ORIGIN_SET);
}