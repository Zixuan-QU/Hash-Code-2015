

#include <cassert>
#include <vector>

#include <iostream>

#include <string>

int min_ham, max_size, x_size, y_size;

using namespace std;

struct box{
    int x_min, y_min, x_max, y_max;

    box(){}
    box(int xmn, int ymn, int xmx, int ymx) : x_min(xmn), y_min(ymn), x_max(xmx), y_max(ymx) {}

    int area() const{ return (x_max - x_min) * (y_max - y_min); }
};

struct gettr{
    std::vector<int> costs;
    std::vector<bool> used, ham;

    int & get_at_pos(int x, int y){
        assert(costs.size() == (x_size+1)*(y_size+1));
        assert(x <= x_size);
        assert(y <= y_size);
        return costs[y*(x_size+1) + x];
    }

    bool occ_at_pos(int x, int y){
        return used[y*x_size + x];
    }
    bool ham_at_pos(int x, int y){
        return ham[y*x_size + x];
    }

    int get_nbr_ham(box B){
        int ret = 0;
        for(int i=B.x_min; i<B.x_max; ++i){
            for(int j=B.y_min; j<B.y_max; ++j){
                if(ham_at_pos(i, j)){
                    ++ret;
                }
            }
        }
        return ret;
        //return get_at_pos(B.x_max, B.y_max) + get_at_pos(B.x_min, B.y_min)
        //     - get_at_pos(B.x_max, B.y_min) - get_at_pos(B.x_min, B.y_max);
    }
    bool unused(box B){
        bool ret = true;
        for(int i=B.x_min; i<B.x_max; ++i){
            for(int j=B.y_min; j<B.y_max; ++j){
                ret = ret && (not occ_at_pos(i, j));
            }
        }
        return ret;
    }
    void use(box B){
        for(int i=B.x_min; i<B.x_max; ++i){
            for(int j=B.y_min; j<B.y_max; ++j){
                used[j*x_size + i] = true;
            }
        }
    }
    int neighbours(box B){
        int ret = 0;
        if(B.x_min == 0){
            ret += (B.y_max - B.y_min);
        }
        else{
            for(int i=B.y_min; i<B.y_max; ++i){
                if(occ_at_pos(B.x_min-1, i)) ++ret;
            }
        }
        if(B.x_max == x_size){
            ret += (B.y_max - B.y_min);
        }
        else{
            for(int i=B.y_min; i<B.y_max; ++i){
                if(occ_at_pos(B.x_max, i)) ++ret;
            }
        }
        if(B.y_min == 0){
            ret += (B.x_max - B.x_min);
        }
        else{
            for(int i=B.x_min; i<B.x_max; ++i){
                if(occ_at_pos(i, B.y_min-1)) ++ret;
            }
        }
        if(B.y_max == y_size){
            ret += (B.x_max - B.x_min);
        }
        else{
            for(int i=B.x_min; i<B.x_max; ++i){
                if(occ_at_pos(i, B.y_max)) ++ret;
            }
        }
        return ret;
    }
};


gettr input_a_gettr(){
    gettr ret;
    cin >> x_size >> y_size >> min_ham >> max_size;

    ret.ham.resize(x_size * y_size, false);

    string dummy;
    getline(cin, dummy);

    for(int i=0; i<x_size; ++i){
        string line;
        getline(cin, line);
        //cout << "Got line: " << line << endl;
        for(int j=0; j<y_size; ++j){
            ret.ham[j*x_size+i] = (line[j]=='H');
        }
    }

    //cout << "Input done " << endl;
    int tot_size = (x_size+1)*(y_size+1);
    ret.used.resize(x_size*y_size, false);
    ret.costs.resize(tot_size, 0);

    for(int i=0; i<=x_size; ++i){
        for(int j=0; j<=y_size; ++j){
            int cur=0;
            for(int k=0; k<i; ++k){
                for(int l=0; l<j; ++l){
                    if(ret.ham[l*x_size+k])
                        ++cur;
                }
            }
            ret.costs[j*(x_size+1)+i] = cur;
        }
    }

    return ret;
}

void output_boxes(vector<box> & boxes){
    cout << boxes.size() << endl;
    for(auto B : boxes){
        std::cout << B.x_min << " " << B.y_min << " " << B.x_max-1 << " " << B.y_max-1 << endl;
    }
}

int main(){
    gettr pizza = input_a_gettr();

    //std::cout << "Got it" << endl;
    vector<box> ret;

    int covered=0;
    int it=0;
    while(true){
        bool found_box = false;
        box best_box;
        float best_cost = -100000.0;
        for(int i=0; i<x_size; ++i){
            for(int j=0; j<y_size; ++j){
                for(int len=1; len<=max_size and i+len <= x_size; ++len){
                    for(int hgh=1; hgh*len<=max_size and j+hgh <= y_size; ++hgh){
                        box cur(i, j, i+len, j+hgh);
                        int ham = pizza.get_nbr_ham(cur);
                        if(ham >= min_ham and pizza.unused(cur)){
                            float cur_cost = ((float) (hgh*len)) / ham + 0.001 * pizza.neighbours(cur) * hgh*len;
                            //float cur_cost = ((float) (hgh*len));
                            if(cur_cost > best_cost){
                                best_box = cur;
                                found_box = true;
                                best_cost = cur_cost;
                            }
                        } 
                    }
                }
            }
        }
        if(it % 100 ==0 or not found_box){
            cout << "Iteration " << it << " with value " << covered << endl;
        }
        if(not found_box) break;
        assert(pizza.unused(best_box));
        assert(best_box.area() <= max_size);
        pizza.use(best_box);
        ret.push_back(best_box);
        covered += best_box.area();

        ++it;
    }


    output_boxes(ret);
}


