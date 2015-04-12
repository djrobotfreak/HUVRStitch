#pragma once
#include <vector>
using namespace std;

class StitchQueue{
public:
    vector<Image*> images;
public:
    void add(Image* image){
        images.push_back(image);
    }
    Image* take(){
        cout<<images.size();
        if (images.size() == 0){
            return NULL;
        }
        Image* popped = images[images.size()-1];
        images.pop_back();
        return popped;
    }
    
};