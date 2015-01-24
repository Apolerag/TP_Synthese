
#include "App.h"

#include "Geometry.h"
#include "Transform.h"

#include "ProgramManager.h"
#include "Image.h"
#include "ImageArray.h"
#include "ImageManager.h"

#include "Mesh.h"
#include "MeshIO.h"

#include "GL/GLTexture.h"
#include "GL/GLSampler.h"
#include "GL/GLBasicMesh.h"


class TP : public gk::App
{
    gk::GLBasicMesh *m_mesh;
    gk::GLProgram *m_program;
    std::vector<gk::GLTexture *> m_textures;
    
    float m_rotate;
    float m_distance;
    
public:
    // creation du contexte openGL et d'une fenetre
    TP( )
        :
        gk::App(),
        m_mesh(NULL),
        m_program(NULL),
        m_textures()
    {
        // specifie le type de contexte openGL a creer :
        gk::AppSettings settings;
        settings.setGLVersion(3,3);     // version 3.3
        settings.setGLCoreProfile();      // core profile
        settings.setGLDebugContext();     // version debug pour obtenir les messages d'erreur en cas de probleme
        
        // cree le contexte et une fenetre de dimensions 1280x768
        createWindow(1280, 768, settings);      
    }
    
    ~TP( ) {}
    
    int init( )
    {
        // charger l'objet
        std::string filename= "bigguy.obj";
        gk::Mesh *mesh= gk::MeshIO::readOBJ(filename);
        if(mesh == NULL)
            return -1;  // erreur de lecture
        
        // shader program
        gk::programPath("shaders");
        m_program= gk::createProgram("texture.glsl");
        if(m_program == gk::GLProgram::null())
            ERROR("program not found\n");
        
        // construit les buffers + vertex array
        m_mesh= new gk::GLBasicMesh(GL_TRIANGLES, mesh->indices.size());
        if(mesh == NULL)
            return -1;
        m_mesh->createBuffer(m_program->attribute("position"), mesh->positions);
        m_mesh->createBuffer(m_program->attribute("normal"), mesh->normals);
        m_mesh->createBuffer(m_program->attribute("texcoord"), mesh->texcoords);
        m_mesh->createIndexBuffer(mesh->indices);       //! un index buffer est cree, ce sera glDrawElements pour dessiner au lieu de glDrawArrays.
        
        // charger les images associees aux matieres.
        gk::imagePath("images"); // indique un repertoire dans lequel chercher les images a charger.
        for(unsigned int i= 0; i < mesh->groups.size(); i++)
        {
            const std::string& texture= mesh->groups[i].material.diffuse_texture;
            if(texture.empty())
                continue;
            
            gk::Image *image= gk::readImage(gk::IOFileSystem::pathname(filename) + texture);    // avec mise en cache
            if(image == NULL)
                image= gk::defaultImage();
            
            m_textures.push_back( (new gk::GLTexture())->createTexture2D(gk::GLTexture::UNIT0, image) );
        }
        
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        delete mesh;
        
        //
        m_distance= 60.f;
        m_rotate= 0.f;
        
        return 0;       // tout c'est bien passe, sinon renvoyer -1
    }
    
    int quit( ) 
    {
        delete m_mesh;
        // le shader program et les shaders sont detruits automatiquement par gKit.
        return 0;
    }
    
    int draw( )
    {
        if(key(SDLK_ESCAPE))
            // fermer l'application si l'utilisateur appuie sur ESCAPE
            closeWindow();

        if(key(SDLK_LEFT))
            m_rotate-= 1.f;
        if(key(SDLK_RIGHT))
            m_rotate+= 1.f;
        if(key(SDLK_UP))
            m_distance+= 1.f;
        if(key(SDLK_DOWN))
            m_distance-= 1.f;
        
        if(key('r'))
        {
            key('r')= 0;
            gk::reloadPrograms();
        }
        
        if(key('w'))
        {
            key('w')= 0;
            
            static int wireframe= 0;
            wireframe = (wireframe + 1) % 2;
            if(wireframe)
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            else
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        
        // transformations
        gk::Transform model= gk::RotateY(30.f); 
        gk::Transform view= gk::Translate( gk::Vector(0.f, 0.f, -m_distance) ) * gk::RotateY(m_rotate);
        gk::Transform projection= gk::Perspective(50.f, 1.f, 1.f, 1000.f);
        
        // composition des transformations
        gk::Transform mv= view * model;
        gk::Transform mvp= projection * mv;
        
        // fixer la transformation viewport en fonction des dimensions de la fenetre
        glViewport(0, 0, windowWidth(), windowHeight());
        // effacer l'image
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // activer le shader
        glUseProgram(m_program->name);
        
        // parametrer le shader : matrices
        m_program->uniform("mvpMatrix")= mvp.matrix();
        m_program->uniform("normalMatrix")= mv.normalMatrix();
        m_program->uniform("color")= gk::Vec4(1.0f, 1.0f, 1.0f, 1.0f);
        
        if(m_textures.size() > 0)
        {
            // textures
            int unit= 0;
            m_program->sampler("diffuse_texture")= unit;
            glBindSampler(unit, gk::defaultSampler()->name);
            
            glActiveTexture(GL_TEXTURE0 + unit);
            glBindTexture(m_textures[0]->target, m_textures[0]->name);
        }
        
        // dessiner
        m_mesh->draw();
        
        // nettoyage, desactive les differents objets selectionnes
        glUseProgram(0);
        glBindVertexArray(0);
        
        // afficher le buffer de dessin
        present();

        if(key('s'))
        {
            key('s')= 0;
            //~ gk::writeFramebuffer("screenshot.png");
            
            // numeroter les screenshots
            static int n= 1;
            char tmp[1024];
            snprintf(tmp, sizeof(tmp), "screenshot%d.png", n);
            gk::writeFramebuffer(tmp);
            n++;
        }
        
        return 1;       // continuer, dessiner une autre fois l'image, renvoyer 0 pour arreter l'application
    }
};


int main( int argc, char **argv )
{
    TP app;
    app.run();
    
    return 0;
}

