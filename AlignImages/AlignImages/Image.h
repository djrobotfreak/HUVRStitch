#include <vector>
#include <string>
#include "Coord.h"
extern string TEMP_PATH;
using namespace std;

class Image{
public:
    bool is_parent;
    Coord coord;
    Coord tl;
    Coord br;
    string loc;
    string name;
    string in_name_1;
    string in_name_2;
    Image* parent;
    vector<Image*> children;
    Image(Coord _coord, float height, float width, string _loc){
        coord = _coord;
        loc = _loc;
        tl = Coord(_coord.x - width/2.0, _coord.y + height/2.0);
        br =Coord(_coord.x + width/2.0, _coord.y - height/2.0);
        is_parent = false;
    }
    Image(std::vector<Image*> _children, string _in1, string _in2, string _name){
        children = _children;
        is_parent = true;
        in_name_1 = _in1;
        in_name_2 = _in2;
        loc = TEMP_PATH + _name + ".jpg";
        name = _name;
    }
    float getIntersectionArea(Image* i){
        if (is_parent){
            float area = 0;
            for(int j = 0; j < children.size(); j++){
                area += children[j]->getIntersectionArea(i);
            }
            return area;
        }
        else if (i->is_parent){
            float area = 0;
            for (int j = 0; j < i->children.size(); j++){
                area += i->children[j]->getIntersectionArea(this);
            }
            return area;
        }
        else{
            if (tl.x > i->br.x || i->tl.x > br.x)
                return 0;
            if (tl.y < i->br.y || i->tl.y < br.y)
                return 0;
            float left = max(tl.x, i->tl.x);
            float right = min(br.x, i->br.x);
            float bottom = max(br.y, i->br.y);
            float top = min(tl.y, i->tl.y);
            return ((right-left) * (top - bottom));
        }
    }
};
