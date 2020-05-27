
#include "Program.h"
#include <iostream>
#include <cassert>
#include <fstream>

#include "GLSL.h"


std::string readFileAsString(const std::string &fileName)
{
	std::string result;
	std::ifstream fileHandle(fileName);

	if (fileHandle.is_open())
	{
		fileHandle.seekg(0, std::ios::end);
		result.reserve((size_t) fileHandle.tellg());
		fileHandle.seekg(0, std::ios::beg);

		result.assign((std::istreambuf_iterator<char>(fileHandle)), std::istreambuf_iterator<char>());
	}
	else
	{
		std::cerr << "Could not open file: '" << fileName << "'" << std::endl;
	}

	return result;
}

void Program::setShaderNames(const std::string &v, const std::string &f)
{
	vShaderName = v;
	fShaderName = f;
}

bool Program::init()
{
	GLint rc;

	// Create shader handles
	GLuint VS = glCreateShader(GL_VERTEX_SHADER);
	GLuint FS = glCreateShader(GL_FRAGMENT_SHADER);

	// Read shader sources
	std::string vShaderString = readFileAsString(vShaderName);
	std::string fShaderString = readFileAsString(fShaderName);
	const char *vshader = vShaderString.c_str();
	const char *fshader = fShaderString.c_str();
	CHECKED_GL_CALL(glShaderSource(VS, 1, &vshader, NULL));
	CHECKED_GL_CALL(glShaderSource(FS, 1, &fshader, NULL));

	// Compile vertex shader
	CHECKED_GL_CALL(glCompileShader(VS));
	CHECKED_GL_CALL(glGetShaderiv(VS, GL_COMPILE_STATUS, &rc));
	if (!rc)
	{
		if (isVerbose())
		{
			GLSL::printShaderInfoLog(VS);
			std::cout << "Error compiling vertex shader " << vShaderName << std::endl;
		}
		return false;
	}

	// Compile fragment shader
	CHECKED_GL_CALL(glCompileShader(FS));
	CHECKED_GL_CALL(glGetShaderiv(FS, GL_COMPILE_STATUS, &rc));
	if (!rc)
	{
		if (isVerbose())
		{
			GLSL::printShaderInfoLog(FS);
			std::cout << "Error compiling fragment shader " << fShaderName << std::endl;
		}
		return false;
	}

	// Create the program and link
	pid = glCreateProgram();
	CHECKED_GL_CALL(glAttachShader(pid, VS));
	CHECKED_GL_CALL(glAttachShader(pid, FS));
	CHECKED_GL_CALL(glLinkProgram(pid));
	CHECKED_GL_CALL(glGetProgramiv(pid, GL_LINK_STATUS, &rc));
	if (!rc)
	{
		if (isVerbose())
		{
			GLSL::printProgramInfoLog(pid);
			std::cout << "Error linking shaders " << vShaderName << " and " << fShaderName << std::endl;
		}
		return false;
	}

	return true;
}

void Program::bind()
{
	CHECKED_GL_CALL(glUseProgram(pid));
}

void Program::unbind()
{
	CHECKED_GL_CALL(glUseProgram(0));
}

void Program::addAttribute(const std::string &name)
{
	attributes[name] = GLSL::getAttribLocation(pid, name.c_str(), isVerbose());
}

void Program::addUniform(const std::string &name)
{
	uniforms[name] = GLSL::getUniformLocation(pid, name.c_str(), isVerbose());
}

GLint Program::getAttribute(const std::string &name) const
{
	std::map<std::string, GLint>::const_iterator attribute = attributes.find(name.c_str());
	if (attribute == attributes.end())
	{
		if (isVerbose())
		{
			std::cout << name << " is not an attribute variable" << std::endl;
		}
		return -1;
	}
	return attribute->second;
}

GLint Program::getUniform(const std::string &name) const
{
	std::map<std::string, GLint>::const_iterator uniform = uniforms.find(name.c_str());
	if (uniform == uniforms.end())
	{
		if (isVerbose())
		{
			std::cout << name << " is not a uniform variable" << std::endl;
		}
		return -1;
	}
	return uniform->second;
}

void setMaterial(int i, std::shared_ptr<Program> prog) {  
	if (!prog->hasTexture) 
		return;
	switch (i) {    
		case 0:
			glUniform3f(prog->getUniform("MatAmb"), 0.2, 0.2, 0.2);        
			glUniform3f(prog->getUniform("MatDif"), 0.0, 0.16, 0.9);       
			glUniform3f(prog->getUniform("MatSpec"), 0.14, 0.2, 0.8);       
			glUniform1f(prog->getUniform("shine"), 120.0);
		break;    
		case 1: // 
			glUniform3f(prog->getUniform("MatAmb"), 0.35, 0.35, 0.35);       
			glUniform3f(prog->getUniform("MatDif"), 0.3, 0.3, 0.35);       
			glUniform3f(prog->getUniform("MatSpec"), 0.3, 0.3, 0.3);       
			glUniform1f(prog->getUniform("shine"), 4.0);
		break;    
		case 2: //pikachu
			glUniform3f(prog->getUniform("MatAmb"), 0.1294, 0.0235, 0.02745);        
			glUniform3f(prog->getUniform("MatDif"), 0.7804, 0.5686, 0.11373);       
			glUniform3f(prog->getUniform("MatSpec"), 0.3922, 0.341176, 0.30784);       
			glUniform1f(prog->getUniform("shine"), 20);
		break;  
		case 3: //stage
			glUniform3f(prog->getUniform("MatAmb"), 0.00745, 0.1175, 0.3175);        
			glUniform3f(prog->getUniform("MatDif"), 0.61424, 0.04136, 0.04136);       
			glUniform3f(prog->getUniform("MatSpec"), 0.727811, 0.626959, 0.626959);       
			glUniform1f(prog->getUniform("shine"), 100);
		break;
		case 4:
			glUniform3f(prog->getUniform("MatAmb"), 0.1, 0.1, 0.1);        
			glUniform3f(prog->getUniform("MatDif"), 0.5, 0.5, 0.5);       
			glUniform3f(prog->getUniform("MatSpec"), .8, 0.8, 0.8);       
			glUniform1f(prog->getUniform("shine"), 100);
		break;
		case 5: // 
			glUniform3f(prog->getUniform("MatAmb"), 0.35, 0.35, 0.35);       
			glUniform3f(prog->getUniform("MatDif"), 0.3, 0.3, 0.3);       
			glUniform3f(prog->getUniform("MatSpec"), 0.3, 0.3, 0.3);       
			glUniform1f(prog->getUniform("shine"), 100.0);
		break;    
	}
}