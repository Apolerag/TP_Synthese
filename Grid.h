#ifndef _GRID_H
#define _GRID_H

#include <vector>
#include <climits>

#include "Vec.h"

//~ typedef gk::TVec3<int> Gridsize;
//~ typedef gk::TVec3<int> Gridpoint;
//~ typedef gk::TVec3<int> Gridindex;

struct Gridpoint : public gk::TVec3<int>
{
    Gridpoint( const int _x= 0, const int _y= 0, const int _z= 0 ) :  gk::TVec3<int>(_x, _y, _z) {}
    
};

struct Gridindex : public gk::TVec3<int>
{
    Gridindex( const int _x= 0, const int _y= 0, const int _z= 0 ) :  gk::TVec3<int>(_x, _y, _z) {}
    
};

struct Gridsize : public gk::TVec3<int>
{
    Gridsize( const int _x= 0, const int _y= 0, const int _z= 0 ) :  gk::TVec3<int>(_x, _y, _z) {}
};


//! boite englobante. bornes inclusives. [min max] x [min max]
struct Gridbox
{
    Gridpoint pMin;
    Gridpoint pMax;
    
    Gridbox( ) : pMin(INT_MAX, INT_MAX, INT_MAX), pMax(INT_MIN, INT_MIN, INT_MIN) {}
    Gridbox( const Gridpoint& pmin, const Gridpoint& pmax ) : pMin(INT_MAX, INT_MAX, INT_MAX), pMax(INT_MIN, INT_MIN, INT_MIN) 
    {
        insert(pmin);
        insert(pmax);
    }
    
    //! renvoie vrai si le point est a l'interieur de la boite. 
    bool inside( const Gridpoint& p ) const
    {
        if(p.x < pMin.x || p.y < pMin.y || p.z < pMin.z)
            return false;
        if(p.x > pMax.x || p.y > pMax.y || p.z > pMax.z)
            return false;
        
        return true;
    }
    
    //! agrandi la boite pour contenir le point.
    void insert( const Gridpoint& p )
    {
        pMin.x= std::min(pMin.x, p.x);
        pMin.y= std::min(pMin.y, p.y);
        pMin.z= std::min(pMin.z, p.z);
        pMax.x= std::max(pMax.x, p.x);
        pMax.y= std::max(pMax.y, p.y);
        pMax.z= std::max(pMax.z, p.z);
    }
    
    //! agrandi la boite pour contenir b.
    void insert( const Gridbox& b )
    {
        pMin.x= std::min(pMin.x, b.pMin.x);
        pMin.y= std::min(pMin.y, b.pMin.y);
        pMin.z= std::min(pMin.z, b.pMin.z);
        pMax.x= std::max(pMax.x, b.pMax.x);
        pMax.y= std::max(pMax.y, b.pMax.y);
        pMax.z= std::max(pMax.z, b.pMax.z);
    }
    
    bool operator== ( const Gridbox& b ) const
    {
        return (pMin == b.pMin && pMax == b.pMax);
    }
};


//! classe de base : representation d'une grille.
struct Grid
{
    Gridsize size;      //!< dimensions de la grille
    Gridsize scale;     //!< facteur d'echelle
    Gridbox bbox;       //!< boite englobante de la grille dans le monde
    
    //! constructeur.
    Grid( const Gridsize& _size ) : size(_size), scale(), bbox() {}
    
    //! constructeur.
    Grid( const Gridsize& _size, const Gridbox& _bbox  ) : size(_size), scale(), bbox(_bbox)
    {
        scale.x= (bbox.pMax.x - bbox.pMin.x +1) / size.x;
        scale.y= (bbox.pMax.y - bbox.pMin.y +1) / size.y;
        scale.z= (bbox.pMax.z - bbox.pMin.z +1) / size.z;
    }
    
    //! constructeur nomme.
    void create( const Gridbox& _bbox )
    {
        bbox= _bbox;
        
        assert(size.x != 0 && size.y != 0 && size.z != 0);
        scale.x= (bbox.pMax.x - bbox.pMin.x +1) / size.x;
        scale.y= (bbox.pMax.y - bbox.pMin.y +1) / size.y;
        scale.z= (bbox.pMax.z - bbox.pMin.z +1) / size.z;
    }
    
    //! renvoie l'indice lineaire du point dans la grille, ou -1 en cas d'erreur (p n'appartient pas a la grille).
    int index( const Gridpoint& p ) const
    {
        if(bbox.inside(p) == false)
            return -1;
        
        return index(grid_index(p));
    }
    
    int index( const Gridindex& i ) const
    {
        assert(!(i.x < 0 || i.y < 0 || i.z < 0));
        assert(!(i.x > size.x || i.y > size.y || i.z > size.z));
        
        return i.y * size.x*size.z + i.x * size.z + i.z;
    }
    
    //! renvoie l'indice 3d du point dans la grille.
    Gridindex grid_index( const Gridpoint& p ) const
    {
        return Gridindex( (p.x - bbox.pMin.x) / scale.x, (p.y - bbox.pMin.y) / scale.y, (p.z - bbox.pMin.z) / scale.z );
    }
    
    //! renvoie un point dans le repere du monde connaissant son index 3d dans la grille.
    Gridpoint grid_point( const Gridindex& index ) const
    {
        return Gridpoint( bbox.pMin.x + index.x * scale.x, bbox.pMin.y + index.y * scale.y, bbox.pMin.z + index.z * scale.z );
    }
    
    //! renvoie un point dans le repere du monde connaissant son index 3d dans la grille.
    Gridpoint grid_point( const int index_x, const int index_y, const int index_z ) const
    {
        return Gridpoint( bbox.pMin.x + index_x * scale.x, bbox.pMin.y + index_y * scale.y, bbox.pMin.z + index_z * scale.z );
    }
};


struct World;
struct Map;
struct Region;
struct Block;


/*! representation du monde : hachage spatial d'un ensemble de maps.
 1 block= 16x16x16 voxels
 1 region = 16x16x16 blocks= 256x256x256 voxels
 1 map= 16x1x16 regions= 4096*256*4096 voxels
 1 world= 16x1x16 maps= 65536x256x65536 voxels
*/
struct World : public Grid
{
    World( ) 
        : 
        // fixe l'etendue du monde
        Grid( Gridsize(16, 1, 16), Gridbox(Gridpoint(-32768, 0, -32768), Gridpoint(32767, 255, 32767)) ),
        maps(16*16, -1), data()
    {}
    
    const Map *map( const Gridpoint& p ) const;
    Map *map( const Gridpoint& p );
    const Region *region( const Gridpoint& p ) const;
    Region *region( const Gridpoint& p );
    const Block *block( const Gridpoint& p ) const;
    Block *block( const Gridpoint& p );
    
    int loadMap( const std::string& filename );
    int loadRegion( const std::string& filename );
    int insert( const Block& block );
    
    std::vector<short> maps;    //!< index spatial
    std::vector<Map> data;      //!< donnees
};

//! representation d'une map : hachage spatial 16x1x16 d'un ensemble de regions.
struct Map : public Grid
{
    Map( ) : Grid(Gridsize(16, 1, 16)), regions(16*16, -1), data() {}
    Map( const Gridbox& _bbox ) : Grid(Gridsize(16, 1, 16), _bbox), regions(16*16, -1), data() {}
    
    const Region *region( const Gridpoint& p ) const;
    Region *region( const Gridpoint& p );
    const Block *block( const Gridpoint& p ) const;
    Block *block( const Gridpoint& p );
    
    std::vector<short> regions; //!< index spatial
    std::vector<Region> data;   //!< donnees
};

//! representation d'une region : hachage spatial 16x16x16 d'un ensemble de blocks.
struct Region : public Grid
{
    Region( ) : Grid(Gridsize(16, 16, 16)), blocks(16*16*16, -1), data() {}
    Region( const Gridbox& _bbox ) : Grid(Gridsize(16, 16, 16), _bbox), blocks(16*16*16, -1), data() {}
    
    const Block *block( const Gridpoint& p ) const;     // renvoie l'ensemble de voxels autour des coordonnees de p, ou NULL en cas d'erreur.
    Block *block( const Gridpoint& p );     // renvoie l'ensemble de voxels autour des coordonnees de p, ou NULL en cas d'erreur.
    
    std::vector<short> blocks;  //!< index spatial
    std::vector<Block> data;    //!< donnees
};

//! representation d'un block : enumeration spatiale de 16x16x16 voxels.
struct Block : public Grid
{
    Block( ) : Grid(Gridsize(16, 16, 16)) {}
    Block( const Gridbox& _bbox ) : Grid(Gridsize(16, 16, 16), _bbox) {}
    
    Block( const Gridbox& _bbox, const std::vector<unsigned char>& voxels ) : Grid(Gridsize(16, 16, 16), _bbox), data(voxels) { assert(voxels.size() == 4096); }
    Block( const Gridbox& _bbox, unsigned char *voxels ) : Grid(Gridsize(16, 16, 16), _bbox), data(voxels, voxels + 4096) { assert(data.size() == 4096); }
    
    int voxel( const Gridpoint& p ) const;
    int voxel( const Gridindex& index )  const;
    
    std::vector<unsigned char> data;    //!< 16x16x16 == 4096 valeurs
};


#endif
