
#include <vector>
#include <algorithm>
#include <cassert>

#include <iostream>

struct server{
    int ind;
    int size;
    int cap;
};

struct alloc_server : server{
    int pool;
    int row;
    int slot;
};

struct row_space{
    int min_slot, len, remaining;
    std::vector<alloc_server> allocateds;

    void add_server(server S, int p, int r){
        assert(S.size <= remaining);
        alloc_server cur;
        cur.ind  = S.ind;
        cur.cap  = S.cap;
        cur.size = S.size;
        cur.pool = p;
        cur.row  = r;
        cur.slot = min_slot + len - remaining;
        remaining -= cur.size;
        allocateds.push_back(cur);
    }
};

struct row{
    std::vector<row_space> places;

    void add_place(int b, int e){
        row_space cur;
        cur.min_slot = b;
        cur.len = e - b;
        cur.remaining = cur.len;
        places.push_back(cur);
    }
};

typedef std::pair<int, int> pr;
void try_add(std::vector<row> & rows, std::vector<std::pair<int, int> > & gcaps, std::vector<std::vector<int> > & pool_allocs, server S){

    struct gain{
        int pool;
        int row;
        int tot_gain;
        int this_gain;
        int cur_cap;

        bool operator<(gain const o) const{
            return tot_gain < o.tot_gain
            or (tot_gain == o.tot_gain and cur_cap >  o.cur_cap)
            or (tot_gain == o.tot_gain and cur_cap == o.cur_cap and this_gain < o.this_gain);
        }
    };

    int old_gcap = std::min_element(gcaps.begin(), gcaps.end(), [](pr a, pr b){return a.first < b.first or (a.first == b.first and a.second < b.second); })->first;

    std::vector<gain> all_row_gains;
    for(int r=0; r<rows.size(); ++r){
        std::vector<gain> row_gains;
        for(int p=0; p<pool_allocs.size(); ++p){
            auto old_val = gcaps[p];

            // Get the gain for this row and this pool
            pool_allocs[p][r] += S.cap;
            gcaps[p].second += S.cap;
            // Update the gcaps
            int max_disr = *std::max_element(pool_allocs[p].begin(), pool_allocs[p].end());
            gcaps[p].first = gcaps[p].second - max_disr;
            int cur_gcap = std::min_element(gcaps.begin(), gcaps.end(), [](pr a, pr b){return a.first < b.first; })->first;

            gain cur_gain;
            cur_gain.pool = p;
            cur_gain.row = r;
            cur_gain.tot_gain = cur_gcap - old_gcap;
            cur_gain.this_gain = gcaps[p].first - old_val.first;
            cur_gain.cur_cap = gcaps[p].second;
            row_gains.push_back(cur_gain);
            
            pool_allocs[p][r] -= S.cap;
            gcaps[p] = old_val;
        }
        all_row_gains.push_back(*std::max_element(row_gains.begin(), row_gains.end()));
    }
    std::sort(all_row_gains.begin(), all_row_gains.end());

    bool row_found = false;
    while(not row_found and not all_row_gains.empty()){

    // We always try to add to the pool with the lowest g_cap;
    //int best_pool = std::min_element(gcaps.begin(), gcaps.end(), [](pr a, pr b){return a.first < b.first or (a.first == b.first and a.second < b.second); }) - gcaps.begin();
    int best_pool = all_row_gains.back().pool;

    auto const & cur_alloc = pool_allocs[best_pool];

    // TODO
    // Better: try for every row except the max one
    //int best_row = std::min_element(cur_alloc.begin(), cur_alloc.end()) - cur_alloc.begin();
    int best_row = all_row_gains.back().row;
    all_row_gains.pop_back();

    auto & cur_row = rows[best_row];
    int best_place=0, best_remaining = 1000000; bool found = false;
    for(int i=0; i<cur_row.places.size(); ++i){
        if(cur_row.places[i].remaining < best_remaining and cur_row.places[i].remaining >= S.size){
            best_place = i;
            found = true;
        }
    }
    if(found){
        cur_row.places[best_place].add_server(S, best_pool, best_row);
        pool_allocs[best_pool][best_row] += S.cap;
        gcaps[best_pool].second += S.cap;
        // Update the gcaps
        int max_disr = *std::max_element(pool_allocs[best_pool].begin(), pool_allocs[best_pool].end());
        gcaps[best_pool].first = gcaps[best_pool].second - max_disr;
        row_found = true;
    }
    }

}

int main(){

    int R, S, U, P, M;
    std::cin >> R >> S >> U >> P >> M;

    std::vector<server> servers;
    std::vector<row> rows(R);

    std::vector<std::vector<int> > unavailables(R);
    for(int i=0; i<U; ++i){
        int row, pos;
        std::cin >> row >> pos;
        unavailables[row].push_back(pos);
    }
    for(int i=0; i<R; ++i){
        auto & un = unavailables[i];
        std::sort(un.begin(), un.end());

        int prev = 0;
        for(int sl : un){
            if(sl > prev){
                rows[i].add_place(prev, sl);
            }
            prev = sl+1;
        }
        rows[i].add_place(prev, S);
    }
    
    for(int i=0; i<M; ++i){
        server cur;
        cur.ind = i;
        std::cin >> cur.size >> cur.cap;
        if(cur.cap > 0)
            servers.push_back(cur);
    }

    // Highest ratio first
    std::sort(servers.begin(), servers.end(), [](server const a, server const b){
        float ratioa = static_cast<float>(a.cap) / a.size, ratiob = static_cast<float>(b.cap) / b.size;
        //return ratioa > ratiob or a.size < b.size;
        //return ratioa*std::sqrt(a.size) > ratiob*std::sqrt(b.size);
        return ratioa*std::pow(a.size, POWER) > ratiob*std::pow(b.size, POWER);
        //return a.size < b.size or (a.size == b.size and  static_cast<float>(a.cap) / a.size > static_cast<float>(b.cap) / b.size) ;
        });

    // Capacities by suppressing a row for each pool
    std::vector<pr> g_caps(P, pr(0, 0));
    std::vector<std::vector<int> > pool_allocs(P, std::vector<int>(R, 0));

    for(server cur_s : servers){
        try_add(rows, g_caps, pool_allocs, cur_s);
    }

    std::cout << "Guaranteed capacity is " << std::min_element(g_caps.begin(), g_caps.end(), [](pr a, pr b){return a.first < b.first; })->first << std::endl;

    // Output
    std::vector<alloc_server> ret;
    for(auto const & cur_row : rows){
        for(auto const & cur_place : cur_row.places){
            for(auto const cur_s : cur_place.allocateds){
                ret.push_back(cur_s);
            }
        }
    }

    // Biggest first
    std::sort(ret.begin(), ret.end(), [](alloc_server const a, alloc_server const b){ return a.ind > b.ind;});

    for(int i = 0; i<M; ++i){
        if(ret.empty() or i != ret.back().ind){
            if(not ret.empty()){
                assert(i < ret.back().ind);
            }
            std::cout << "x" << std::endl;
        }
        else{
            auto cur_s = ret.back();
            std::cout << cur_s.row << " " << cur_s.slot << " " << cur_s.pool << std::endl;
            ret.pop_back();
        }
    }

    return 0;
}

