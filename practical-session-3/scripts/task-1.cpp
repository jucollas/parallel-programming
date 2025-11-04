#include <bits/stdc++.h>
#include <pthread.h>
#include <cstdio>


using namespace std;
namespace fs = filesystem;

int N_THREADS = 8;
const string folder_out = "gray_images";
const string folder_in = "images";
const string extend_in = ".ppm";
const string extend_out = ".pgm";

struct data_t{
  vector<string>* namefiles;
  int start, end;
  data_t(vector<string>* _files, int _start, int _end){
    namefiles = _files;
    start =  _start;
    end = _end;
  }
  data_t(){
    namefiles = NULL;
    start = -1;
    end = -1;
  }
};

class RGB{
  public:
    int r, g, b;
    RGB(int _r, int _g, int _b) : r(_r), g(_g), b(_b) {}
};

bool read_file(string& namefile, vector<vector<RGB>>& image){
    FILE* file = fopen(namefile.c_str(), "r");
    if (file == nullptr) {
        std::cerr << "Error: incorrect path or file not found." << std::endl;
        return false;
    }

    char magic_buf[3];
    if (fscanf(file, "%2s", magic_buf) != 1) {
        std::cerr << "Error reading magic number." << std::endl;
        fclose(file);
        return false;
    }
    int width, height;
    if (fscanf(file, "%d %d", &width, &height) != 2) {
        std::cerr << "Error reading width/height." << std::endl;
        fclose(file);
        return false;
    }
    int max_value;
    if (fscanf(file, "%d", &max_value) != 1) {
        std::cerr << "Error reading max value." << std::endl;
        fclose(file);
        return false;
    }

    int pixel_count = width * height;
    int pixel_count_file = pixel_count * 3;
    int* data = (int*) malloc(pixel_count_file * sizeof(int));
    if (!data) {
        std::cerr << "Memory allocation error." << std::endl;
        fclose(file);
        return false;
    }

    for (int i = 0; i < pixel_count_file; i++) {
        if (fscanf(file, "%d", &data[i]) != 1) {
            std::cerr << "Error reading pixels." << std::endl;
            free(data);
            fclose(file);
            return false;
        }
    }

    fclose(file);

    int ind = 0;
    for(int i = 0; i < width; ++i){
      image.push_back({});
      for(int j = 0; j < height; ++j){
        image[i].push_back(RGB(data[ind], data[ind + 1], data[ind + 2]));
        ind += 3;
      }
    }

    free(data); 
    return true;

}

int transform(RGB pix){
  return static_cast<int>(0.299 * pix.r + 0.587 * pix.g + 0.114 * pix.b);;
}

vector<vector<int>> gray_filter(vector<vector<RGB>>& image){
  int n = image.size(), m = image[0].size();
  vector<vector<int>> gray(n, vector<int>(m));
  for(int i = 0; i < n; ++i){
    for(int j = 0; j < m; ++j){
      gray[i][j] = transform(image[i][j]);
    }
  }
  return gray;
}

bool write_file(string& namefile, vector<vector<int>>& image){
  FILE* file = fopen(namefile.c_str(), "w+");
    if (file == nullptr) {
        std::cerr << "Error: incorrect path or file not found." << std::endl;
        return false;
    }

    char magic[] = "P2";
    int max_val = 255;

    fprintf(file, "%s\n", magic);
    fprintf(file, "%d %d\n", (int) image.size(), (int) image[0].size());
    fprintf(file, "%d\n", max_val);
    for(int i = 0; i < (int) image.size(); ++i){
      for(int j = 0; j < (int) image[i].size(); ++j){
        fprintf(file, "%d\n", image[i][j]);
      }
    }
    fclose(file);
    return true;
}


string get_noum(string& file){
  int n = file.size();
  int ind = n - 1;
  while (ind >= 0 && file[ind] != '.'){
    --ind;
  }
  return string(file.begin(), file.begin() + ind);
}

void apply_filter_single(string& namefile){
  vector<vector<RGB>> image;
  if(read_file(namefile, image)){
    vector<vector<int>> image_gray = gray_filter(image);
    string out_file = "gray_" + get_noum(namefile) + extend_out;
    if(write_file(out_file, image_gray)){
      cout << "Succefull :" << out_file << endl;
    }
  }
}

void* apply_filter_mult(void* args){
  data_t * data = (data_t *) args;
  //cout << data->start << " " << data->end << endl;
  for(int i = data->start; i < data->end; ++i){
    apply_filter_single((*data->namefiles)[i]);
  }
  return NULL;
}


void apply_filter_set_images(vector<string>& files){
  int n = (int) files.size();
  int num_threads = min(n, N_THREADS);
  pthread_t threads[num_threads];
  data_t tdata[num_threads];

  int len_chunk = n / num_threads;
  int remaider = n % num_threads;
  int curr = 0;
  for(int i = 0; i < num_threads; ++i){
    int mv = i < remaider ? 1 : 0;
    int to = curr + mv + len_chunk;
    tdata[i] = data_t(&files, curr, to);
    pthread_create(&threads[i], NULL, apply_filter_mult, &tdata[i]);
    curr = to;
  }
  
  for(int i = 0; i < num_threads; ++i){
    pthread_join(threads[i], NULL);
  }
}

bool endsWith(const std::string& str, const std::string& suffix) {
    if (suffix.size() > str.size()) return false;
    return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
}

vector<string> list_folder(const string& path){
  vector<string> ans;
  for (const auto & entry : fs::directory_iterator(path)){
    string path_file = entry.path();
    if(endsWith(path_file, extend_in)){
      ans.push_back(path_file);
    }
  }
  return ans;
}

int main(){
  vector<string> namefiles = list_folder(folder_in);
  for( int i = 0; i < namefiles.size(); ++i){
    cout << namefiles[i] << endl;
  }
  if (filesystem::create_directory(folder_out)) {
    cout << "Directory '" << folder_out << "' created successfully." << endl;
  }else {
    cout << "Failed to create directory '" << folder_out << "' or it already exists." << endl;
  }
  apply_filter_set_images(namefiles);
  return 0;
}