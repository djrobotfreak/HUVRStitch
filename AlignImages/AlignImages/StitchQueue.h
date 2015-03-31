#pragma once
#include <vector>
using namespace std;

class StitchQueue{
private:
    vector<Image*> images;
public:
    void add(Image* image){
        images.push_back(image);
    }
    Image* take(){
        if (images.size() == 0){
            return NULL;
        }
        Image* popped = images[images.size()-1];
        images.pop_back();
        return popped;
    }
    
};