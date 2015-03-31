//
//  main.cpp
//  AlignImages
//
//  Created by Derek J Wene on 2/17/15.
//  Copyright (c) 2015 Derek Wene. All rights reserved.
//
#include <exiv2/exiv2.hpp>
#include <iostream>
#include <vector>
#include <math.h>
#include <cmath>
#include <fstream>
#include <string>
#include <sstream>
#include "Image.h"
#include "Coord.h"
#include "StitchQueue.h"
#include <pthread.h>
#include <semaphore.h>
#include <iomanip>
#include <cassert>



using namespace std;

template <typename T> string tostr(const T& t) {
    ostringstream os;
    os<<t;
    return os.str();
}

StitchQueue* CURRENT_QUEUE;
sem_t queue_lock;
float WIDTH = 10;
float HEIGHT = 8;
float THRESHOLD = 5;
int curr_num = 0;
vector<StitchQueue*> stitch_queue;
string generate_name(){
    curr_num ++;
    string name = "stitched_" + tostr(curr_num);
    return name;
}
vector<Image*> ALL_IMAGES;
vector<Image*> load_images(string loc){
    ifstream source;
    vector<Image*> image_list;
    string name = "";
    source.open(loc.c_str(), ios_base::in);
    for(string line; getline(source, line);){
        istringstream in(line);
        float x;
        float y;
        string name;
        in >> x;
        in >> y;
        in >> name;
        Coord a = Coord(x, y, 180);
        image_list.push_back(new Image(a, HEIGHT, WIDTH, name));
    }
    ALL_IMAGES = image_list;
    return image_list;
}

Image* stitch(Image* i1, Image* i2, int iter){
    string i1_name = i1->name;
    string i2_name = i2->name;
    vector<Image*> children;
    if(i1->is_parent){
        for(int i = 0; i < i1->children.size(); i++){
            children.push_back(i1->children[i]);
        }
    }
    else{
        children.push_back(i1);
    }
    if(i2->is_parent){
        for(int i = 0; i < i2->children.size(); i++){
            children.push_back(i2->children[i]);
        }
    }
    else{
        children.push_back(i2);
    }
    Image* out = new Image(children,i1_name, i2_name, generate_name());
    stitch_queue[iter]->add(out);
    ALL_IMAGES.push_back(out);
    return out;
}

vector<Image*> find_pairs(vector<Image*> list, int iter){
    vector<Image*> new_list;
    while(list.size() > 0){
        Image* current = list[0];
        unsigned int highest_index = -1;
        float highest_area = 0;
        for (unsigned int i = 1; i < list.size(); i++){
            float temp_area = list[0]->getIntersectionArea(list[i]);
            if(temp_area > highest_area){
                highest_index = i;
                highest_area = temp_area;
            }
        }
        if (highest_area > THRESHOLD){
            new_list.push_back(stitch(current, list[highest_index], iter));
            list.erase(list.begin()+highest_index);
            list.erase(list.begin());
        }
        else{
            new_list.push_back(current);
            list.erase(list.begin());
        }
    }
    return new_list;
}

void loadAndPairImages(){
    vector<Image*> image_list = load_images("/Users/Derek/Dropbox/HUVRData/StitchingScript/inputGrid.txt");
    string location = "/Users/Derek/Dropbox/HUVRData/StitchingScript/Photos/";
    vector<string> old_image_list;
    string output = location + "\n" + "BEGIN\n";
    int iter = 0;
    while(image_list.size()> 1 || iter < 1){
        old_image_list.clear();
        for (int i = 0; i< image_list.size(); i++){
            old_image_list.push_back(image_list[i]->name);
        }
        stitch_queue.push_back(new StitchQueue());
        image_list = find_pairs(image_list, iter);
        if (image_list.size() == old_image_list.size()){
            cout<<"No remaining pairs were found.\n";
            break;
        }
        cout<<"Beginning Iteration "<<iter<<"\n\n";
        for (int i = 0; i < image_list.size(); i++){
            bool repeat = false;
            cout<<image_list[i]->name;
            for (int j = 0; j < old_image_list.size(); j++){
                if (old_image_list[j] == image_list[i]->name){
                    cout<<" not adding :(  "<<old_image_list[j] <<"\t"<<image_list[i]->name;
                    repeat = true;
                    break;
                }
            }
            if (!repeat){
                cout<<"\tadding!!";
                output = output + image_list[i]->in_name_1 + " " + image_list[i]->in_name_2 + " " + image_list[i]->name + "\n";
            }
            cout<<endl;
        }
        cout<<"\nEnd of Iteration: "<<iter<<"\n";
        output = output + "END_ITER " + tostr(iter) +"\n";
        iter++;
    }
    output = output + "END\n";
    for (unsigned short i = 0; i < stitch_queue.size(); i++){
        cout<<"Iter\n";
        Image* current = stitch_queue[i]->take();
        while(current != NULL){
            cout<<current->name<<endl;
            current = stitch_queue[i]->take();
        }
    }
    output = output + "\nFINAL IMAGES:\n";
    for (int i = 0 ;i < image_list.size(); i++){
        output += image_list[i]->name + "\n";
    }
    cout<<output;
    cout<<"Final image count: "<<image_list.size()<<endl;
    ofstream out_file (location + "data.stitch");
    if (out_file.is_open())
    {
        out_file << output;
        out_file.close();
    }
    else cout << "Unable to open file";

}

void * stitch_thread(void* arg){
    while(true){
        sem_wait(&queue_lock);
        Image* image = CURRENT_QUEUE->take();
        if (image == NULL){
            pthread_exit(NULL);
        }
        sem_post(&queue_lock);
        string name = image->name;
        string pto_name = image->name+".pto";
        string create_pto ="pto_gen -o " + pto_name + " -p 0 -f 10 " +image->in_name_1 + ".jpg " + image->in_name_2+ ".jpg";
        system(create_pto.c_str());
        string sift = "autopano-sift-c " + pto_name + " " + pto_name;
        system(sift.c_str());
        string create_vars = "pto_var --opt y,p,r -o " + pto_name + " " + pto_name;
        system(create_vars.c_str());
        string optimize = "autooptimiser -n -o " + pto_name + " " + pto_name;
        system(optimize.c_str());
        string modify = "pano_modify -o " + pto_name + " --center --straighten --canvas=AUTO --crop=AUTO " + pto_name;
        system(modify.c_str());
        string make = "pto2mk -o "+ pto_name + ".mk -p " + name + "_final " +pto_name;
        system(modify.c_str());
        string run = "make -f " + pto_name + ".mk";
        system(run.c_str());
        
//            pto_gen -o to_stitch.pto -p 0 -f 10 $1 $2
//            autopano-sift-c output.pto to_stitch.pto
//            echo "\n\ncreating variables\n\n"
//            pto_var --opt y,p,r -o output2.pto output.pto
//            echo "\n\noptimisation 1\n\n"
//            autooptimiser -n -o output3.pto output2.pto
//        # echo "\n\noptimisation 2\n\n"
//        # autooptimiser -m -o output4.pto output3.pto
//            echo "\n\nstarting pano_modify\n\n"
//        # pano_modify -o output5.pto --center --straighten --canvas=AUTO --crop=AUTO output3.pto
//            pano_modify -o output5.pto --center --fov=AUTO --canvas=AUTO output3.pto
//            echo "\n\nstarting pto2mk\n\n"
//            pto2mk -o output.pto.mk -p output_stuff output5.pto
//            echo "\n\ndone creating make, see final. now making.\n\n"
//            make -f output.pto.mk
//            sips -s format png output_stuff.tif -o $3
//            sudo rm *.tif
    }
}

int main(int argc, const char * argv[]){
    string in = "ATM.jpg";
    Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(in);
    assert(image.get() != 0);
    image->readMetadata();
    
    Exiv2::ExifData &exifData = image->exifData();
    if (exifData.empty()) {
        std::string error(in);
        error += ": No Exif data found in the file";
        throw Exiv2::Error(1, error);
    }

//    loadAndPairImages();
//    for (int i = 0; i < stitch_queue.size(); i++){
//        CURRENT_QUEUE = stitch_queue[i];
//        pthread_t t1, t2, t3, t4;
//        pthread_create( &t1, NULL, &stitch_thread, NULL);
//        pthread_create( &t2, NULL, &stitch_thread, NULL);
//        pthread_create( &t3, NULL, &stitch_thread, NULL);
//        pthread_create( &t4, NULL, &stitch_thread, NULL);
//        pthread_join( t1, NULL);
//        pthread_join( t2, NULL);
//        pthread_join( t3, NULL);
//        pthread_join( t4, NULL);
//
//    }
    cout<<"TERMINATING PROGRAM!\n";
    return 0;
}


