//
// Created by enzoc on 25/10/23.
//

#ifndef EDA_SSTREE_H
#define EDA_SSTREE_H

#include <vector>
#include <algorithm>
#include <iostream>
#include <queue>
#include <limits>
#include <fstream>

#include "params.h"
#include "Point.h"

struct Pair {
    Point point;
    NType distance;

    Pair(const Point& p, NType d) : point(p), distance(d) {}
};
struct Comparator {
    bool operator()(const Pair& a, const Pair& b) const {
        return a.distance < b.distance; // max-heap basado en distancia
    }
};


struct CompareSafe {
    bool operator()(const std::pair<NType, std::string >& a, const std::pair<NType, std::string>& b) {
        return a.first > b.first;
    }
};


class SsNode {
private:
    int D;
    NType varianceAlongDirection(const std::vector<Point>& centroids, size_t direction) const{
        NType sum = 0;
        for (const Point& i : centroids) {
            sum += i[direction];
        }
        NType mean = sum / centroids.size();

        NType variance = 0;
        for (const Point& i : centroids) {
            NType diff = i[direction] - mean;
            variance += diff * diff;
        }
        variance /= centroids.size();
        return variance;
    }
    size_t minVarianceSplit( const std::vector<NType>& values ,size_t coordinateIndex);
public:
    virtual ~SsNode() = default;
    void DFirst(const Point &center, size_t k, std::priority_queue<std::pair<NType, std::string>, std::vector<std::pair<NType, std::string>>, CompareSafe> &result, NType& maxi, std::vector<std::string>& paths);

    Point centroid;
    NType radius;
    SsNode* parent = nullptr;


    virtual bool isLeaf() const = 0;
    virtual std::vector<Point> getEntriesCentroids() const = 0;
    virtual void sortEntriesByCoordinate(size_t coordinateIndex) = 0;
    virtual std::pair<SsNode*, SsNode*> split() = 0;
    virtual bool intersectsPoint(const Point& point) const {
        return distance(this->centroid, point) <= this->radius;
    }

    virtual void updateBoundingEnvelope() = 0;
    size_t directionOfMaxVariance() const;
    size_t findSplitIndex();

    virtual std::pair<SsNode*,SsNode*> insert(const Point& point) = 0;
    virtual std::pair<SsNode*,SsNode*> insert(const Point& point, std::string path) = 0;

    bool test(bool isRoot = false) const;
    void print(size_t indent = 0) const;

    virtual void FNDFTrav(const Point& q, size_t k, std::priority_queue<Pair, std::vector<Pair>, Comparator>& L, NType& Dk) const = 0;



    virtual void saveToStream(std::ostream &out) const = 0;
    virtual void loadFromStream(std::istream &in) = 0;
};

class SsInnerNode : public SsNode {
private:
    int D;
    std::vector<Point> getEntriesCentroids() const override;
    void sortEntriesByCoordinate(size_t coordinateIndex) override;

public:
    SsInnerNode(int DD ){this->D = DD;}
    std::pair<SsNode*, SsNode*> split() override;
    std::vector<SsNode*> children;

    SsNode* findClosestChild(const Point& target) const;
    bool isLeaf() const override { return false; }
    void updateBoundingEnvelope() override;
    std::vector<Point> findFarthestPointsInInnerNode(const Point& pp) ;
    std::pair<SsNode*,SsNode*>  insert(const Point& point) override;
    std::pair<SsNode*,SsNode*>  insert(const Point& point, std::string path) override;

    void FNDFTrav(const Point& q, size_t k, std::priority_queue<Pair, std::vector<Pair>, Comparator>& L, NType& Dk) const override{
        std::vector<SsNode*> A;
        for(auto &i: this->children){
            A.push_back(i);
        }
        // Los elementos de A, estan ordenados en orden ascendente usando MeanOrPivotDIST
        if(k == 1){
            Point Mp = this->centroid;
            for(auto &i : A){
                if(distance(q,Mp) > Dk){ break;}
                else if(distance(q,Mp) + radius < Dk) {

                }
            }
        }


    }

    virtual void saveToStream(std::ostream &out) const override;
    virtual void loadFromStream(std::istream &in) override;
};

class SsLeaf : public SsNode {
private:
    int D;
    std::vector<Point> getEntriesCentroids() const override;
    void sortEntriesByCoordinate(size_t coordinateIndex) override;

public:
    SsLeaf(int DD){this->D = DD;}
    std::vector<std::string> paths;

    std::pair<SsNode*, SsNode*> split() override;
    std::vector<Point> points;
    Point findFarthestPointInLeaf(const Point &cc) ;

    bool isLeaf() const override { return true; }
    void updateBoundingEnvelope() override;
    std::pair<SsNode*,SsNode*>  insert(const Point& point, std::string path) override;
    std::pair<SsNode*,SsNode*>  insert(const Point& point) override;
    Point calculateMean() const {
        if (this->points.empty()) {
            return {};
        }

        Point sum = this->points[0];
        for (size_t i = 1; i < this->points.size(); ++i) {
            sum += this->points[i];
        }

        return sum / this->points.size();
    }

    void INSERTL(Point* e, NType s, size_t k, std::priority_queue<Pair, std::vector<Pair>, Comparator>& L, NType& Dk) {
        if (L.size() == k) {
            L.pop(); // Elimina el elemento de mayor prioridad
        }

        L.emplace(*e, s); // Inserta el objeto e con prioridad s

        if (L.size() == k) {
            Dk = L.top().distance; // Actualiza Dk con la nueva mayor distancia en L
        }
    }


    void FNDFTrav(const Point& q, size_t k, std::priority_queue<Pair, std::vector<Pair>, Comparator>& L, NType& Dk) const override{
        // Point& q, size_t k, queue<Pair>& L, NType& Dk
        Point M = calculateMean();
        for(auto &o : this->points ){
            if(distance(q,M) - distance(o,M) > Dk ){
                continue;
            }else if(distance(o,M) - distance(q,M) > Dk){
                continue;
            }else{
                NType distt = distance(q,o);
                if(distt < Dk){
//                    INSERTL();
                }
            }
        }
    }

    virtual void saveToStream(std::ostream &out) const override;
    virtual void loadFromStream(std::istream &in) override;
};


class SsTree {
private:
    SsNode* root;
    SsNode* search(SsNode* node, const Point& target);
    SsNode* searchParentLeaf(SsNode* node, const Point& target);
    int D;
public:
    SsTree() : root(nullptr) {}
    SsTree(int D){root = nullptr; this->D = D;}
    ~SsTree() {
        delete root;
    }

    SsNode* getRoot(){return this->root;}
    void insert(const Point& point);
    void insert(const Point& point, const std::string& path);
    void build (const std::vector<Point>& points);
    std::vector<std::string> kNNQuery(const Point& center, size_t k) ;

    void print() const;
    bool test() const;

    void saveToFile(const std::string &filename) const;
    void loadFromFile(const std::string &filename);
};



#endif //EDA_SSTREE_H
