// squelette de lancer de rayons

#include <cstdlib>
#include <time.h>
#include <math.h>       /* cos */

#define PI 3.14159265

#include "Geometry.h"
#include "Transform.h"

#include "Triangle.h"

#include "Mesh.h"
#include "Image.h"

#include "MeshIO.h"
#include "ImageIO.h"


// objet
gk::Mesh *mesh= NULL;

// representation d'une source de lumiere
struct Source
{
    gk::Triangle triangle;
    gk::Color emission;
    
    Source( const gk::Triangle& t, const gk::Color& e ) : triangle(t), emission(e) {}
};

// ensemble de sources de lumieres
std::vector<Source> sources;

// recuperer les sources de lumiere du mesh : triangles associee a une matiere qui emet de la lumiere, material.emission != 0
int build_sources( const gk::Mesh *mesh )
{
    for(int i= 0; i < mesh->triangleCount(); i++)
    {
        // recupere la matiere associee a chaque triangle de l'objet
        const gk::MeshMaterial& material= mesh->triangleMaterial(i);
       
        if(gk::Color(material.emission).isBlack() == false)
            // inserer la source de lumiere dans l'ensemble.
            //std::cout<<gk::Color(material.emission)<<std::endl;
            sources.push_back( Source(mesh->triangle(i), gk::Color(material.emission)) );

    }
    
    printf("%u sources.\n", sources.size());
    return sources.size();
}


// ca
// ensemble de triangles
std::vector<gk::Triangle> triangles;

// recuperer les triangles du mesh
int build_triangles( const gk::Mesh *mesh )
{
    for(int i= 0; i < mesh->triangleCount(); i++) {
        triangles.push_back( mesh->triangle(i) );

        //const gk::MeshMaterial& material= mesh->triangleMaterial(i);
        //gk::Vector c = gk::Normalize(gk::Vector(0.7,0.7,0.7));

        //std::cout<<material.name<<" "<<material.diffuse_texture<<" "<<material.specular_texture<<std::endl;
        // std::cout<<"c.r = "<<c.x<<" c.g = "<<c.y<<" c.b = "<<c.z<<std::endl;

        // std::cout<<"kd = "<<material.kd<<" ks = "<<material.ks<<" ns = "<<material.ns<<" ni = "<<material.ni<<std::endl;
    }
        
    
    printf("%u triangles.\n", triangles.size());
    return triangles.size();
}


// calcule l'intersection d'un rayon et de tous les triangles
bool intersect( const gk::Ray& ray, gk::Hit& hit, bool ombre = false )
{
    for(unsigned int i= 0; i < triangles.size(); i++) {
    	float t, u, v;
    	if(triangles[i].Intersect(ray, hit.t, t, u, v)) {
                if(ombre)
                    return true;
                hit.t= t;
                hit.u= u;
                hit.v= v;
                hit.p= ray(t);      // evalue la positon du point d'intersection sur le rayon
                hit.object_id= i;
    	}
    }

    return (hit.object_id != -1);
}

float oneFloat( )
{
    return drand48();
}

double distance(const gk::Point &p, const gk::Point &q)
{
    return sqrt( pow(p.x - q.x, 2) + pow(p.y - q.y, 2) + pow(p.z - q.z, 2) );
}

/**
 * @brief calcul l'éclairage direct d'un point
 * @details 
 * 
 * @param p 
 * @param n 
 * @param material 
 * @param nbRayon le nombre de rayon à lancer
 * @param proportionnelle le choix de la repartition des sources de lumières 
 * @return l'eclairage direct de p
 */
gk::Color direct( const gk::Point& p, const gk::Normal& n, const gk::MeshMaterial& material , const unsigned int nbRayon = 1000, const bool proportionnelle = true)
{
    gk::Color color(0.f,0.f,0.f);
    const float r = material.diffuse_color.r, g = material.diffuse_color.g, b = material.diffuse_color.b;
    const float kd = material.kd;
   
   if(!proportionnelle) {
        //repartition égale des rayons par sources de lumières
        #pragma omp parallel for schedule(dynamic,1)
        for (unsigned int i = 0; i < sources.size(); ++i)
        {
            for (unsigned int j = 1; j < (nbRayon+1)/sources.size(); ++j)
            {
                gk::Point pointSources;
                float proba = sources[i].triangle.sampleUniform(oneFloat(),oneFloat(),pointSources); // choisi un point aléatoirement sur le triangle   
                                                                                                    //et retourne la probabilité que le point soit choisi
                gk::Ray ray(p, pointSources);  // construire le rayon entre le point et la source
                gk::Hit hit(ray);   // preparer l'intersection

                if(!intersect(ray, hit,true)) { // si pas d'objet entre la source de lumière et la point 
                    color += gk::Color(r/nbRayon, g/nbRayon, b/nbRayon) * cos( -gk::Dot(n, gk::Normalize(p-pointSources)));
                }
            }
        }
        #pragma omp barrier
    }
    else {

        //repartition des rayons proportionnelle à la tailles des sources de lumières

        double surfaceTotale(0.0);
        //std::vector<double> v.resize(sources.size());

        for (unsigned int i = 0; i < sources.size(); ++i)
            surfaceTotale += sources[i].triangle.area();

        #pragma omp parallel for schedule(dynamic,1)
        for (unsigned int i = 0; i < sources.size(); ++i)
        {
            double aire = sources[i].triangle.area();
            for (unsigned int j = 1; j < aire/surfaceTotale*nbRayon; ++j)
            {
                gk::Point pointSources;
                float proba = sources[i].triangle.sampleUniform(oneFloat(),oneFloat(),pointSources); // choisi un point aléatoirement sur le triangle   
                                                                     //et retourne la probabilité que le point soit choisi
                gk::Ray ray(p, pointSources);  // construire le rayon entre le point et la source
                gk::Hit hit(ray);   // preparer l'intersection
                

                if(!intersect(ray, hit,true)) { // si pas d'objet entre la source de lumière et la point 
                    color += gk::Color(r/nbRayon, g/nbRayon, b/nbRayon) * cos( -gk::Dot(n, gk::Normalize(p-pointSources)));
                }
            }
        }
        #pragma omp barrier
    }
    return color;
}

/**
 * @brief permet le calcul de l'eclairage indirecte 
 * @details 
 * 
 * @param p 
 * @param n 
 * @param material 
 * @param nbRayon le nombre de rayons à lancer à partir de p 
 * @param nbLance le nombre de rebond du rayon à calculer
 * @return l'éclairage indirect de p.
 */
gk::Color indirect( const gk::Point& p, const gk::Normal& n, const gk::MeshMaterial& material, const unsigned int nbRayon = 50, const int nbLance = 2 )
{
    const float r = material.diffuse_color.r, g = material.diffuse_color.g, b = material.diffuse_color.b;
    const float kd = material.kd;

    gk::Color color(0.f, 0.f, 0.f);

    if(nbLance == 0)
        return gk::Color(0.f,0.f,0.f);

    #pragma omp parallel for schedule(dynamic,1)
    for (unsigned int i = 0; i < nbRayon; ++i)
    {
        gk::Ray ray(p, gk::Vector(oneFloat(),oneFloat(),oneFloat()) );  // construire un rayon partant de p
        gk::Hit hit(ray);   // preparer l'intersection

        if(intersect(ray, hit)) { // si un objet est touché par le rayon

            gk::Point pHit = ray(hit.t);
            gk::Normal normalHit = mesh->triangle(hit.object_id).normal();
            gk::MeshMaterial materialHit = mesh->triangleMaterial(hit.object_id);
            
            if(!(p == pHit)) {

                color += gk::Color(r/nbRayon, g/nbRayon, b/nbRayon) * cos( -gk::Dot(n, gk::Normalize(p-pHit)) );
                color += indirect(pHit, normalHit, materialHit, nbRayon, nbLance-1)/(2*nbRayon);
            }
        }
    }
    #pragma omp barrier

    return color;
}

int main( )
{
    srand48( time(NULL) );
    
    // charger un objet
    mesh= gk::MeshIO::readOBJ("geometry.obj");
    if(mesh == NULL) return 1;

    build_sources(mesh);        // recupere les sources de lumiere
    build_triangles(mesh);      // recupere les triangles
   
    // creer une image resultat
    gk::Image *image= gk::createImage(512, 512);
    //gk::Image *imageProp= gk::createImage(512, 512);
   
    // definir les transformations
    gk::Transform model;
    
    /*
        geometry
            translate x y z
            -221.766296 232.837692 575.962341 
            rotate y x
            -378.000000 -7.000000
    */
    
    gk::Transform view= (gk::Translate( gk::Vector(-221.f, 232.f, 575.f) ) * gk::RotateX(-7.f) * gk::RotateY(-378.f)).inverse();
    gk::Transform projection= gk::Perspective(50.f, 1.f, 1.f, 1000.f);  // projection perspective
    gk::Transform viewport= gk::Viewport(image->width, image->height);      // transformation adaptee a la resolution de l'image resultat
    
    // compose les transformations utiles
    gk::Transform vpv= viewport * projection * view;
    
    // parcours tous les pixels de l'image
    for(int y= 0; y < image->height; y++)
    {
        for(int x= 0; x < image->width; x++)
        {
            // generer le rayon pour le pixel (x,y) dans le repere de l'image
            gk::Point origine(x +.5f, y + .5f, -1.f);    // sur le plan near
            gk::Point extremite(x +.5f, y + .5f, 1.f);    // sur le plan far
           
            // transformer le rayon dans le repere de la scene
            gk::Point o= vpv.inverse(origine); // transformation de origine
            gk::Point e= vpv.inverse(extremite); // transformation de extremite
            
            gk::Ray ray(o, e);  // construire le rayon
            gk::Hit hit(ray);   // preparer l'intersection
            
            gk::Color color(0.f,0.f,0.f);    // couleur du pixel. 
           
            // calculer l'intersection du rayon avec les triangles
            if(intersect(ray, hit))
            {
                // si une intersection existe... hit_object_id= indice du triangle
                // recuperer les informations sur la matiere associee au point d'intersection
                gk::Point p= ray(hit.t);
                gk::Normal normal= mesh->triangle(hit.object_id).normal();
                gk::MeshMaterial material= mesh->triangleMaterial(hit.object_id);

                // couleur "aleatoire", eventuellement
                material.diffuse_color= gk::Color(material.diffuse_color) * gk::Color(1.f - float(hit.object_id % 100) / 99.f, float(hit.object_id % 10) / 9.f, float(hit.object_id % 1000) / 999.f);       
                
                // calculer l'energie reflechie par le point vers la camera
                // etape 1 : eclairage direct
                color += direct(p, normal, material,100,true);
               
                // etape 2 : eclairage indirect
                color += indirect(p, normal, material, 20, 2);
            }
           
            // ecrire la couleur dans l'image
            image->setPixel(x, y, gk::Color(color.r, color.g, color.b, 1.0f));
        }
    }
   
    // enregistrer l'image
    gk::ImageIO::writeImage("render.png", image);
    delete image;
    
    return 0;
}
