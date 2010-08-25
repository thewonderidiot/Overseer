#include "Overseer.h"

int main( int argc, char** argv )
{
    Overseer *overseer = new Overseer();
    overseer->go();
    delete overseer;
    return 0;
}
