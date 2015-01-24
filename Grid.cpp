
#include <string>
#include <cstdio>
#include <cassert>

#include "Grid.h"

// acces aux donnees
const Map *World::map( const Gridpoint& p ) const
{
    int id= index(p);
    if(id < 0 || maps[id] < 0)
        return NULL;
    
    return &data[maps[id]];
}

Map *World::map( const Gridpoint& p ) 
{
    int id= index(p);
    if(id < 0 || maps[id] < 0)
        return NULL;
    
    return &data[maps[id]];
}

const Region *World::region( const Gridpoint& p ) const
{
    const Map *m= map(p);
    if(m == NULL)
        return NULL;
    
    return m->region(p);
}

Region *World::region( const Gridpoint& p )
{
    Map *m= map(p);
    if(m == NULL)
        return NULL;
    
    return m->region(p);
}

const Block *World::block( const Gridpoint& p ) const
{
    const Map *m= map(p);
    if(m == NULL)
        return NULL;
    const Region *r= m->region(p);
    if(r == NULL)
        return NULL;
    
    return r->block(p);
}

Block *World::block( const Gridpoint& p )
{
    Map *m= map(p);
    if(m == NULL)
        return NULL;
    Region *r= m->region(p);
    if(r == NULL)
        return NULL;
    
    return r->block(p);
}

const Region *Map::region( const Gridpoint& p ) const
{
    int id= index(p);
    if(id < 0 || regions[id] < 0)
        return NULL;
    
    return &data[regions[id]];
}

Region *Map::region( const Gridpoint& p )
{
    int id= index(p);
    if(id < 0 || regions[id] < 0)
        return NULL;
    
    return &data[regions[id]];
}

const Block *Map::block( const Gridpoint& p ) const
{
    const Region *r= region(p);
    if(r == NULL)
        return NULL;
    
    return r->block(p);
}

Block *Map::block( const Gridpoint& p )
{
    Region *r= region(p);
    if(r == NULL)
        return NULL;
    
    return r->block(p);
}

const Block *Region::block( const Gridpoint& p ) const
{
    int id= index(p);
    if(id < 0 || blocks[id] < 0)
        return NULL;
    
    return &data[blocks[id]];
}

Block *Region::block( const Gridpoint& p )
{
    int id= index(p);
    if(id < 0 || blocks[id] < 0)
        return NULL;
    
    return &data[blocks[id]];
}

int Block::voxel( const Gridpoint& p ) const
{
    int id= index(p);
    if(id < 0)
        return -1;      // ou 0 ??
    
    return (int) data[id];
}

int Block::voxel( const Gridindex& i ) const
{
    int id= index(i);
    if(id < 0)
        return -1;      // ou 0 ??
    
    return (int) data[id];
}


// construction du hachage spatial
int World::insert( const Block& block )
{
    // inserer le bloc dans le monde
    Gridpoint p(block.bbox.pMin);
    int id= index(p);
    if(id < 0)
        return -1;
    
    if(maps[id] < 0)
    {
        // creer une nouvelle map
        Gridindex index= grid_index(p);
        Gridpoint mmin( grid_point(index) );
        Gridpoint mmax( mmin.x + scale.x -1, mmin.y + scale.y -1, mmin.z + scale.z -1 );
        
        Gridbox mbox(mmin, mmax);
        assert(mbox.inside(p));
        maps[id]= (short) data.size();
        data.push_back( Map(mbox) );
    }
    
    Map *m= &data[maps[id]];
    // inserer le bloc dans la map
    int mid= m->index(p);
    if(mid < 0)
        return -1;
    
    if(m->regions[mid] < 0)
    {
        // creer une nouvelle region
        Gridindex index= m->grid_index(p);
        Gridpoint rmin( m->grid_point(index) );
        Gridpoint rmax( rmin.x + m->scale.x -1, rmin.y + m->scale.y -1, rmin.z + m->scale.z -1 );
        
        Gridbox rbox(rmin, rmax);
        assert(rbox.inside(p));
        m->regions[mid]= (short) m->data.size();
        m->data.push_back( Region(rbox) );
    }
    
    Region *r= &m->data[m->regions[mid]];
    // inserer le bloc dans la region
    int rid= r->index(p);
    if(rid < 0)
        return -1;
    
    if(r->blocks[rid] < 0)
    {
        // verifier les bbox
        Gridindex index= r->grid_index(p);
        Gridpoint bmin( r->grid_point(index) );
        Gridpoint bmax( bmin.x + r->scale.x -1, bmin.y + r->scale.y -1, bmin.z + r->scale.z -1 );
        
        Gridbox bbox(bmin, bmax);
        assert(bbox.inside(p));
        assert(bbox == block.bbox);
        
        // inserer le nouveau block
        r->blocks[rid]= (short) r->data.size();
        r->data.push_back( block );
        return 0;
    }
    
    // le block existe deja...
    return -1;
}


// chargement des donnees
int World::loadMap( const std::string& pathname )
{
    std::string filename= pathname + ".txt";
    printf("loading map '%s'...\n", filename.c_str());
    
    FILE *in= fopen(filename.c_str(), "rt");
    if(in == NULL)
    {
        printf("loading map '%s'... failed\n", filename.c_str());
        return -1;
    }
    
    bool error= true;
    char tmp[1024];
    for(;;)
    {
        if(fscanf(in, " %[^\r\n] ", tmp) != 1)
        {
            error= false;
            break;
        }
        
        if(tmp[0] == '#')
            // saute les commentaires
            continue;
        
        if(loadRegion(pathname + "/" + tmp) < 0)
            break;
    }
    
    fclose(in);
    return error ? -1 : 0;
}
    
int World::loadRegion( const std::string& filename )
{
    printf("loading region '%s'...\n", filename.c_str());
    
    int x, z;
    if(sscanf(filename.c_str(), "%*[^.].%d.%d.gkmc", &x, &z) != 2)
    {
        printf("loading region '%s'... failed.\n", filename.c_str());
        return -1;
    }
    
    // 1 region = 16*16*16 blocks
    // 1 block = 16*16*16 voxels
    // 1 region = 256*256*256 voxels
    Gridbox rbox( Gridpoint(x*256, 0, z*256), Gridpoint(x*256 + 255, 255, z*256 + 255) );
    
    printf("  bbox %d, %d, %d  %d, %d, %d\n", rbox.pMin.x, rbox.pMin.y, rbox.pMin.z, rbox.pMax.x, rbox.pMax.y, rbox.pMax.z);
    
    FILE *in= fopen(filename.c_str(), "rb");
    if(in == NULL)
        return -1;
    
    int size= 0;        // nombre de blocks dans la region
    if(fread(&size, sizeof(int), 1, in) != 1)
        return -1;
    
    // indexation des blocks
    std::vector<short> blocks(4096, -1);
    if(fread(&blocks.front(), sizeof(short), 4096, in) != 4096)
        return -1;
    
    // donnees des blocks
    std::vector<unsigned char> voxels(size, 0);
    if(fread(&voxels.front(), sizeof(unsigned char), size, in) != (size_t) size)
        return -1;
    
    fclose(in);
    
    // cree les blocks et les insere dans le hachage spatial 
    int i= 0;
    for(int by= 0; by < 16; by++)
    for(int bz= 0; bz < 16; bz++)
    for(int bx= 0; bx < 16; bx++, i++)
        if(blocks[i] != -1)
        {
            // identifie la position des donnees du block
            unsigned long int offset= (unsigned long int) blocks[i];
            assert(offset * sizeof(char[16*16*16]) + 4096 <= (unsigned int) size);
            
            // boite englobante du block
            Gridpoint vmin(rbox.pMin.x + bx*16, rbox.pMin.y + by *16, rbox.pMin.z + bz*16);
            Gridpoint vmax(vmin.x + 15, vmin.y + 15, vmin.z + 15);
            
            // insere le bloc dans le hachage spatial 
            insert( Block(Gridbox(vmin, vmax), &voxels.front() + offset * sizeof(char[16*16*16])) );
        }
        
    return 0;
}
