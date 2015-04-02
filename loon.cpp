

#include <cassert>
#include <vector>

#include <iostream>

#include <string>

#include <cstdlib>

using namespace std;

typedef pair<int, int> pos;

bool is_legal(pos cur){
    return (cur.first >=0) and (cur.second >= 0);
}

struct world{
    vector<vector<vector<int> > > x_winds, y_winds;
    vector<vector<int> > targets;
    int x_s, y_s;
    int x_b, y_b;
    int alts;
    int radius;
    int balloon_cnt, target_cnt;
    int simu_len;

    int legalize_y(int x){
        while(x < 0){
            x+=y_s;
        }
        while(x >= y_s){
            x-=y_s;
        }
        return x;
    }

    int is_target(pos cur_pos){
        if(cur_pos.first < 0 or cur_pos.first >= x_s) return 0;
        else{
            return targets[cur_pos.first][legalize_y(cur_pos.second)];
        }
    }

    void unset_target(pos cur_pos){
        if(cur_pos.first >= 0 and cur_pos.first < x_s)
            targets[cur_pos.first][legalize_y(cur_pos.second)] = 0;
    }
    pos validated_pos(pos cur_pos){
        if(cur_pos.first >= x_s or cur_pos.first < 0){
            return pos(-1, -1);
        }
        else{
            return pos(cur_pos.first, legalize_y(cur_pos.second));
        }
    } 

    pos next_pos(int alt, pos cur_pos){
        if(not is_legal(cur_pos)) return cur_pos;
        pos ret;
        ret.first = cur_pos.first + x_winds[alt][cur_pos.first][cur_pos.second];
        ret.second = cur_pos.second + y_winds[alt][cur_pos.first][cur_pos.second];
        return validated_pos(ret);
    }

    int cover(pos cur_pos){
        int ret=0;
        if(not is_legal(cur_pos)) return 0;
        for(int x=-radius; x<=radius; ++x){
            for(int y=-radius; y<=radius; ++y){
                if(x*x+y*y <= radius*radius){
                    ret+= is_target(pos(cur_pos.first+x, cur_pos.second+y));
                }
            }
        }
        return ret;
    }

    int covered(std::vector<pos> const & positions){
        vector<vector<int> > coverage(x_s, vector<int>(y_s, 0));
        for(pos const cur_pos : positions){
            if( not is_legal(cur_pos)) continue;
            for(int x=-radius; x<=radius; ++x){
                for(int y=-radius; y<=radius; ++y){
                    if(x*x+y*y <= radius*radius){
                        auto opos = validated_pos(pos(cur_pos.first + x, cur_pos.second + y));
                        if(is_legal(opos)){
                            coverage[opos.first][opos.second] = is_target(opos);
                        }
                    }
                }
            }
        }
        int ret=0;
        for(auto const & R : coverage){
            for(int C : R){
                if(C > 0)
                    ret += C;
            }
        }
        return ret;
    }

    vector<vector<int> > weighted_coverage(std::vector<pos> const & positions){
        vector<vector<int> > coverage(x_s, vector<int>(y_s, 0));
        for(pos const cur_pos : positions){
            if( not is_legal(cur_pos)) continue;
            for(int x=-radius; x<=radius; ++x){
                for(int y=-radius; y<=radius; ++y){
                    if(x*x+y*y <= radius*radius){
                        auto opos = validated_pos(pos(cur_pos.first + x, cur_pos.second + y));
                        if(is_legal(opos)){
                            coverage[opos.first][opos.second] = is_target(opos);
                        }
                    }
                }
            }
        }
        return coverage;
    }

    int next_cover(std::vector<int> const & alt, std::vector<pos> const & positions, std::vector<int> const & changes){
        auto new_positions = positions;
        auto new_alt = alt;
        for(int j=0; j<alt.size(); ++j){
            new_alt[j] = alt[j] + changes[j];
            new_positions[j] = next_pos(new_alt[j], positions[j]);
        }
        return covered(new_positions);
    }

    int chose_change(int alt, pos cur_pos){
        int best_change = 0;
        bool found_feasible=false;
        int this_cover = cover(cur_pos);
        int best_movement = 0;
        for(int c=-1; c<=1; ++c){
            if(alt + c < 0 or alt + c >= alts) continue;
            auto nxt = next_pos(alt+c, cur_pos);
            if(is_legal(nxt)){
                int cur_movement = abs(nxt.first - cur_pos.first) + abs(nxt.second - cur_pos.second);
                if(not found_feasible){
                    best_change = c;
                    best_movement = cur_movement;
                }
                else{
                    if( (this_cover == 0 and cur_movement > best_movement and rand()%2 == 0)
                        or rand()%3 == 0){
                        best_change = c;
                        best_movement = cur_movement;
                    }
                }
                found_feasible = true;
            }
        }
        return best_change; 
    }

    std::vector<int> generate_changes(std::vector<int> const & alt, std::vector<pos> const & positions){
        std::vector<int> ret;
        for(int i=0; i<alt.size(); ++i){
/*            int best_change = 0;
            bool found_feasible=false;
            for(int c=-1; c<=1; ++c){
                if(alt[i] + c < 0 or alt[i] + c >= alts) continue;
                auto nxt = next_pos(alt[i]+c, positions[i]);
                if(is_legal(nxt)){
                    if(not found_feasible){
                        best_change = c;
                    }
                    else{
                        if(rand()%2)
                            best_change = c;
                    }
                    found_feasible = true;
                }
            }
            ret.push_back(best_change);
*/
            ret.push_back(chose_change(alt[i], positions[i]));
        }
        return ret;
    }

    std::vector<int> best_changes(std::vector<int> const & alt, std::vector<pos> const & positions){
        std::vector<int> ret = generate_changes(alt, positions);
        int best_cover = next_cover(alt, positions, ret);
        for(int i=0; i<100; ++i){
            //auto cur_ret = generate_changes(alt, positions);
            auto cur_ret = ret;
            for(int j=0; j<cur_ret.size(); ++j){
                if(!(rand()%10)){
                    cur_ret[j] = chose_change(alt[j], positions[j]);
                }
            }
            int cur_cover = next_cover(alt, positions, cur_ret);
            if(cur_cover > best_cover){
                ret = cur_ret;
                best_cover = cur_cover;
            }
        }
        return ret;
    }

    void use(pos cur_pos){
        for(int x=-radius; x<=radius; ++x){
            for(int y=-radius; y<=radius; ++y){
                if(x*x+y*y <= radius*radius){
                    unset_target(pos(cur_pos.first+x, cur_pos.second+y));
                }
            }
        }
    }
};

world input_world(){
    world ret;

    cin >> ret.x_s >> ret.y_s >> ret.alts;
    cin >> ret.target_cnt >> ret.radius >> ret.balloon_cnt >> ret.simu_len;

    cin >> ret.x_b >> ret.y_b;
    ret.x_winds.resize(ret.alts);
    ret.y_winds.resize(ret.alts);

    ret.targets.resize(ret.x_s, vector<int>(ret.y_s, 0));
    for(int i=0; i<ret.target_cnt; ++i){
        pos cur;
        cin >> cur.first >> cur.second;
        ++ret.targets[cur.first][cur.second];
    }

    for(int a=0; a<ret.alts; ++a){
        ret.x_winds[a].resize(ret.x_s);
        ret.y_winds[a].resize(ret.x_s);

        for(int x=0; x<ret.x_s; ++x){
            ret.x_winds[a][x].resize(ret.y_s);
            ret.y_winds[a][x].resize(ret.y_s);
            for(int y=0; y<ret.y_s; ++y){
                cin >> ret.x_winds[a][x][y] >> ret.y_winds[a][x][y];
            }
        }
    }

    return ret;
}
/*

int main(){
    world wd = input_world();

    std::vector<int> altitudes(wd.balloon_cnt, -1);
    std::vector<pos> positions(wd.balloon_cnt, pos(wd.x_b, wd.y_b));
    std::vector<bool> dead(wd.balloon_cnt, false);

    //srand(time(NULL));
    int total_coverage = 0;

    for(int i=0; i<wd.simu_len; ++i){
        auto changes = wd.best_changes(altitudes, positions);
        for(int j=0; j<wd.balloon_cnt; ++j){
            cout << changes[j];
            if(j+1 < wd.balloon_cnt) cout << " ";
            altitudes[j] += changes[j];
            positions[j] = wd.next_pos(altitudes[j], positions[j]);
            if(not is_legal(positions[j])) dead[j] = true;
        }
        cout << endl;
        //cout << "Coverage is " << wd.covered(positions) << endl;
        total_coverage += wd.covered(positions);
    }
    cout << "Total: " << total_coverage << endl;
    return 0;

}
*/

struct path_elt{
    int best_cover;
    pos prev_pos;
    pos cur_pos;
    int prev_alt;
    int prev_change;

    bool operator>(path_elt const o) const{
        return best_cover > o.best_cover;
    }
};

int main(){
    world wd = input_world();

    //vector<vector<int> > altitudes(wd.balloon_cnt, -1);
    //vector<vector<pos> positions(wd.balloon_cnt, pos(wd.x_b, wd.y_b));
    //vector<vector<vector<int> > > free_targets;

    path_elt basic_pt_e;
    basic_pt_e.best_cover = -1;
    auto basic_path = vector<vector<vector<path_elt> > >(wd.alts, vector<vector<path_elt> >(wd.x_s, vector<path_elt>(wd.y_s, basic_pt_e)));

    //srand(time(NULL));
    auto all_targets = wd.targets;
    vector<vector<int> > alt_changes(wd.simu_len);

    vector<vector<vector<int> > > step_targets(wd.simu_len, all_targets);
    for(int j=0; j<wd.balloon_cnt; ++j){
        cout << "Balloon " << j << endl;
        vector<vector<vector<vector<path_elt> > > > best_cover_on_path; // time, alt, x, y
        best_cover_on_path.push_back(basic_path);
        for(int i=0; i<wd.simu_len; ++i){
            cout << "Timestep " << i << endl;
            wd.targets = step_targets[i];
            vector<vector<vector<path_elt> > > cur_cover_on_path = basic_path;
            cur_cover_on_path[0][wd.x_b][wd.y_b].prev_alt = -1;
            cur_cover_on_path[0][wd.x_b][wd.y_b].prev_change = 1;
            cur_cover_on_path[0][wd.x_b][wd.y_b].best_cover = wd.cover(pos(wd.x_b, wd.y_b));
            cur_cover_on_path[0][wd.x_b][wd.y_b].cur_pos = pos(wd.x_b, wd.y_b);

            if(i>=1){

            auto & old_cover_on_path = best_cover_on_path.back();
            for(int a=0; a<wd.alts; ++a){
                for(int x=0; x<wd.x_s; ++x){
                    for(int y=0; y<wd.y_s; ++y){
                        path_elt nw_pe;
                        nw_pe.best_cover = -1;
                        nw_pe.prev_pos = pos(x, y);
                        for(int c=-1; c<=1; ++c){
                            if(a-c >= 0 and a-c < wd.alts){
                                if(old_cover_on_path[a-c][x][y].best_cover > nw_pe.best_cover){
                                    nw_pe.prev_alt = a-c;
                                    nw_pe.prev_change = c;
                                    nw_pe.best_cover = old_cover_on_path[a-c][x][y].best_cover;
                                }
                            }
                        }
                        auto opos = wd.next_pos(a, pos(x, y));
                        if(is_legal(opos) and nw_pe.best_cover >= 0){
                            nw_pe.best_cover = wd.cover(opos) + nw_pe.best_cover;
                            nw_pe.cur_pos = opos;
                            if(nw_pe > cur_cover_on_path[a][opos.first][opos.second])
                                cur_cover_on_path[a][opos.first][opos.second] = nw_pe;
                        }
                    }
                }
            }
            } // End test i>0
            best_cover_on_path.push_back(cur_cover_on_path);
        }

        path_elt bst_path;
        bst_path.best_cover = -1;
        // Find best destination
        for(int a=0; a<wd.alts; ++a){
            for(int x=0; x<wd.x_s; ++x){
                for(int y=0; y<wd.y_s; ++y){
                    if(best_cover_on_path.back()[a][x][y] > bst_path){
                        bst_path = best_cover_on_path.back()[a][x][y];
                        //bst_path.best_cover = best_cover_on_path.back()[a][x][y].best_cover;
                        //bst_path.prev_pos = pos(x, y);
                    }
                }
            }
        }

        // Backtrack
        bool found_decoll = false;
        for(int i=wd.simu_len; i > 0; --i){
            if(found_decoll){
                alt_changes[i-1].push_back(0);
            }
            else{
                //if(bst_path.best_cover < 0) cout << "Problem!" << endl;
                //cout << "At " << i-1 << ": " << bst_path.best_cover << " at " << bst_path.prev_pos.first << " " << bst_path.prev_pos.second << " with alt " << bst_path.prev_alt << " changing " << bst_path.prev_change << endl;
                alt_changes[i-1].push_back(bst_path.prev_change);
                if(bst_path.prev_alt < 0){
                    //cout << "Position at found pos is " << bst_path.prev_pos.first << " " << bst_path.prev_pos.second << endl;
                    found_decoll = true;
                }
                else{
                    // Cover the zone
                    wd.targets = step_targets[i-1];
                    wd.use(bst_path.cur_pos);
                    step_targets[i-1] = wd.targets;
                    bst_path = best_cover_on_path[i-1][bst_path.prev_alt][bst_path.prev_pos.first][bst_path.prev_pos.second];
                }
            }
        }
        /*
        int alt=-1;
        cout << "Balloon " << j << "'s altitude: ";
        for(int i=0; i<alt_changes.size(); ++i){
            alt += alt_changes[i][j];
            if(alt > wd.alts){
                cout << endl << "Problem!!!" << endl;
            }
            std::cout << alt << " ";
        }
        cout << endl;
        */
    }

    assert(alt_changes.size() == wd.simu_len);
    assert(alt_changes[0].size() == wd.balloon_cnt);
    for(int i=0; i<alt_changes.size(); ++i){
        for(int j=0; j<alt_changes[i].size(); ++j){
            cout << alt_changes[i][j];
            if(j+1 < alt_changes[i].size()){
                cout << " ";
            }
        }
        cout << endl;
    }
    return 0;

}


