#ifndef MODEL_H
#define MODEL_H

#include <GLAD/glad.h>

#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <stb_image.h>
#include <ASSIMP/Importer.hpp>
#include <ASSIMP/scene.h>
#include <ASSIMP/postprocess.h>

#include <NoVarYe/mesh.h>
#include <NoVarYe/shader.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
using namespace std;

//从文件中读取纹理
unsigned int TextureFromFile(const char *path, const string &directory, bool gamma = false);
//Model类
class Model
{
public:
	/*  Model数据 */
	//存储到目前为止加载的所有纹理，优化以确保纹理不会被加载多次。
	vector<Texture> textures_loaded;
	vector<Mesh> meshes;
	string directory;
	bool gammaCorrection;
 
	/*  函数  */
	// 构造汉化,需要一个3D模型的文件路径
	Model(string const &path, bool gamma = false) : gammaCorrection(gamma)
	{
		loadModel(path);
	}
 
	// 绘制模型，从而绘制所有网格
	void Draw(Shader shader)
	{
		for (unsigned int i = 0; i < meshes.size(); i++)
			meshes[i].Draw(shader);
	}
 
private:
	/*  函数   */
	// 从文件加载支持ASSIMP扩展的模型，并将生成的网格存储在网格矢量中。
	void loadModel(string const &path)
	{
		// 通过ASSIMP读文件
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
		// 检查错误
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // 如果不是0
		{
			cout << "错误::ASSIMP:: " << importer.GetErrorString() << endl;
			return;
		}
		// 检索文件路径的目录路径
		directory = path.substr(0, path.find_last_of('/'));
 
		// 以递归方式处理ASSIMP的根节点
		processNode(scene->mRootNode, scene);
	}
 
	// 以递归方式处理节点。 处理位于节点处的每个单独网格，并在其子节点（如果有）上重复此过程。
	void processNode(aiNode *node, const aiScene *scene)
	{
		// 处理位于当前节点的每个网格
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			// 节点对象仅包含索引用来索引场景中的实际对象。
			// 场景包含所有数据，节点只是为了有组织的保存东西（如节点之间的关系）。
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			meshes.push_back(processMesh(mesh, scene));
		}
		// 在我们处理完所有网格（如果有的话）后，我们会递归处理每个子节点
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			processNode(node->mChildren[i], scene);
		}
	}
 
	Mesh processMesh(aiMesh *mesh, const aiScene *scene)
	{
		// 要填写的数据
		vector<Vertex> vertices;
		vector<unsigned int> indices;
		vector<Texture> textures;
 
        // walk through each of the mesh's vertices
        for(unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
            // positions
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;
            // normals
            if (mesh->HasNormals())
            {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.Normal = vector;
            }
            // texture coordinates
            if(mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
            {
                glm::vec2 vec;
                // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
                // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
                vec.x = mesh->mTextureCoords[0][i].x; 
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
                // tangent
                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.Tangent = vector;
                // bitangent
                vector.x = mesh->mBitangents[i].x;
                vector.y = mesh->mBitangents[i].y;
                vector.z = mesh->mBitangents[i].z;
                vertex.Bitangent = vector;
            }
            else
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);

            vertices.push_back(vertex);
        }
		//现在遍历每个网格面（一个面是一个三角形的网格）并检索相应的顶点索引。
		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			// 检索面的所有索引并将它们存储在索引向量中
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}
		// 加工材料
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		// 我们假设着色器中的采样器名称约定。 每个漫反射纹理应命名为'texture_diffuseN'，其中N是从1到MAX_SAMPLER_NUMBER的序列号。
		//同样适用于其他纹理，如下列总结：
		// diffuse: texture_diffuseN
		// specular: texture_specularN
		// normal: texture_normalN
 
		// 1. 漫反射贴图
		vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		// 2. 高光贴图
		vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
		// 3.法线贴图
		std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
		textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
		// 4. 高度贴图
		std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
		textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
 
		// 返回从提取的网格数据创建的网格对象
		return Mesh(vertices, indices, textures);
	}
 
	// 检查给定类型的所有材质纹理，如果尚未加载纹理，则加载纹理。
	// 所需信息作为Texture结构返回。
	vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName)
	{
		vector<Texture> textures;
		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
		{
			aiString str;
			mat->GetTexture(type, i, &str);
			// 检查之前是否加载了纹理，如果是，则继续下一次迭代：跳过加载新纹理
			bool skip = false;
			for (unsigned int j = 0; j < textures_loaded.size(); j++)
			{
				if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
				{
					textures.push_back(textures_loaded[j]);
					skip = true; 
					break;// 已加载具有相同文件路径的纹理，继续下一个（优化）。
				}
			}
			if (!skip)
			{   // 如果尚未加载纹理，请加载它
				Texture texture;
				texture.id = TextureFromFile(str.C_Str(), this->directory);
				texture.type = typeName;
				texture.path = str.C_Str();
				textures.push_back(texture);
				textures_loaded.push_back(texture);  //将其存储为整个模型加载的纹理，以确保我们不会加载重复纹理。
			}
		}
		return textures;
	}
};
//从文件读取纹理函数
unsigned int TextureFromFile(const char *path, const string &directory, bool gamma)
{
	string filename = string(path);
	filename = directory + '/' + filename;
 
	unsigned int textureID;
	glGenTextures(1, &textureID);
 
	int width, height, nrComponents;
	unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;
 
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
 
		stbi_image_free(data);
	}
	else
	{
		std::cout << "纹理无法从此路径加载: " << path << std::endl;
		stbi_image_free(data);
	}
	return textureID;
}
#endif
