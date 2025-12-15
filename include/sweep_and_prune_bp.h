#include "AABB.h"
#include "Collider.h"

struct Endpoint{
    float value;
    Collider* collider;
    bool isMin;
};
class AABBSorter{
public:
    enum Axis{ X, Y, Z };

    explicit AABBSorter(Axis axis): m_axis(axis){}

    void addCollider(Collider* collider){
        Endpoint minEp, maxEp;
        extract(collider->getAABB(),minEp.value,maxEp.value);

        minEp.collider = collider;
        minEp.isMin = true;

        maxEp.collider = collider;
        maxEp.isMin = false;

        m_endpoints.push_back(minEp);
        m_endpoints.push_back(maxEp);
    }

    // every frame
    void update(){
        for(Endpoint& e : m_endpoints){
            const AABB b = e.collider->getAABB();
            e.value = e.isMin ? getMin(b) : getMax(b);
        }

        incrementalSort();
    }

    // Broad-phase overlap generation
    void computePairs(std::vector<std::pair<Collider*, Collider*>>& outPairs){
        outPairs.clear();
        
        std::vector<Collider*> active;

        for(const Endpoint& e : m_endpoints){
            if(e.isMin) {
                for(Collider* other : active){
                    outPairs.emplace_back(e.collider, other);
                }
                active.push_back(e.collider);
            }else{
                erase(active, e.collider);
            }
        }
    }
private:
    Axis m_axis;
    std::vector<Endpoint> m_endpoints;

    // helpers ---------------------

    void extract(const AABB& b, float& outMin, float& outMax) const {
        outMin = getMin(b);
        outMax = getMax(b);
    }

    float getMin(const AABB& b) const {
        switch(m_axis){
            case X: return b.min.x;
            case Y: return b.min.y;
            case Z: return b.min.z;
        }
        return 0.0f;
    }
    float getMax(const AABB& b) const {
        switch(m_axis){
            case X: return b.max.x;
            case Y: return b.max.y;
            case Z: return b.max.z;
        }
        return 0.0f;
    }

    // Insertion sort
    void incrementalSort(){
        for(size_t i = 1; i < m_endpoints.size(); i++){
            size_t j = i;
            while(j>0 && m_endpoints[j].value < m_endpoints[j-1].value){
                std::swap(m_endpoints[j], m_endpoints[j-1]);
                j--;
            }
        }
    }

    static void erase(std::vector<Collider*>& v, Collider* col){
        for(size_t i = 0; i<v.size(); i++){
            if(v[i] == col){
                v[i] = v.back();
                v.pop_back();
                return;
            }
        }
    }
};