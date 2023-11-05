#include "SStree.h"



bool SsNode::test(bool isRoot) const {
    size_t count = 0;
    if (this->isLeaf()) {
        const SsLeaf* leaf = dynamic_cast<const SsLeaf*>(this);
        count = leaf->points.size();

        // Verificar si los puntos están dentro del radio del nodo
        for (const Point& point : leaf->points) {
            if (distance(this->centroid, point) > this->radius) {
                std::cout << "Point outside node radius detected." << std::endl;
                return false;
            }
        }
    } else {
        const SsInnerNode* inner = dynamic_cast<const SsInnerNode*>(this);
        count = inner->children.size();

        // Verificar si los centroides de los hijos están dentro del radio del nodo padre
        for (const SsNode* child : inner->children) {
            if (distance(this->centroid, child->centroid) > this->radius) {
                std::cout << "Child centroid outside parent radius detected." << std::endl;
                return false;
            }
            // Verificar recursivamente cada hijo
            if (!child->test()) {
                return false;
            }
        }
    }

    // Comprobar la validez de la cantidad de hijos/puntos
    if (!isRoot && (count < Settings::m || count > Settings::M)) {
        std::cout << "Invalid number of children/points detected." << std::endl;
        return false;
    }

    // Comprobar punteros de parentezco, salvo para el nodo raíz
    if (!isRoot && !parent) {
        std::cout << "Node without parent detected." << std::endl;
        return false;
    }

    return true;
}

bool SsTree::test() const {
    bool result = root->test(true);

    if (root->parent) {
        std::cout << "Root node parent pointer is not null!" << std::endl;
        result = false;
    }

    if (result) {
        std::cout << "SS-Tree is valid!" << std::endl;
        return true;
    } else {
        std::cout << "SS-Tree has issues!" << std::endl;
        return false;
    }
}


void SsNode::print(size_t indent) const {
    for (size_t i = 0; i < indent; ++i) {
        std::cout << "  ";
    }

    // Imprime información del nodo.
    std::cout << "Centroid: " << centroid << ", Radius: " << radius;
    if (isLeaf()) {
        const SsLeaf* leaf = dynamic_cast<const SsLeaf*>(this);
        std::cout << ", Points: [ ";
        for (const Point& p : leaf->points) {
            std::cout << p << " ";
        }
        std::cout << "]";
    } else {
        std::cout << std::endl;
        const SsInnerNode* inner = dynamic_cast<const SsInnerNode*>(this);
        for (const SsNode* child : inner->children) {
            child->print(indent + 1);
        }
    }
    std::cout << std::endl;
}

size_t SsNode::minVarianceSplit(const std::vector<NType> &values, size_t coordinateIndex) {
    NType minVariance = NType::max_value();
    size_t splitIndex = Settings::m;
    std::vector<Point> gg = this->getEntriesCentroids();

    for(size_t i = Settings::m; i < (gg.size() - Settings::m) ; i ++){
//            std::vector<NType> values1(values.begin(), values.begin() + i);
//            std::vector<NType> values2(values.begin() + i, values.end());
        std::vector<Point> values1(gg.begin(), gg.begin() + i);
        std::vector<Point> values2(gg.begin() + i, gg.end());
        NType variance1 = varianceAlongDirection(values1, coordinateIndex);
        NType variance2 = varianceAlongDirection(values2, coordinateIndex);
        if(variance1 + variance2 < minVariance){
            minVariance = variance2 + variance1;
            splitIndex = i;
        }
    }
    return splitIndex;
}

size_t SsNode::directionOfMaxVariance() const {
    NType maxVariance = 0;
    size_t directionIndex = 0 ;
    std::vector<Point> ce =  this->getEntriesCentroids();
    size_t k = ce[0].dim();
    for(size_t i = 0 ; i < k  ; i ++){
        NType currentVariance = varianceAlongDirection(ce, i);
        if(currentVariance > maxVariance){
            maxVariance = currentVariance;
            directionIndex = i ;
        }
    }
    return directionIndex;
}

size_t SsNode::findSplitIndex() {
    size_t coordinateIndex = this->directionOfMaxVariance();
    this->sortEntriesByCoordinate(coordinateIndex);
    std::vector<NType> coordinates ;
    for(const Point &i : getEntriesCentroids() ){
        coordinates.push_back(i[coordinateIndex]);
    }
    return minVarianceSplit(coordinates, coordinateIndex);
}

void SsTree::print() const {
    if (root) {
        root->print();
    } else {
        std::cout << "Empty tree." << std::endl;
    }
}







void SsLeaf::saveToStream(std::ostream &out) const {
    // Guardar centroid
    centroid.saveToFile(out, D);

    // Guardar el radio
    float radius_ = radius.getValue();
    out.write(reinterpret_cast<const char*>(&radius_), sizeof(radius_));

    // Guardar el numero de puntos
    size_t numPoints = points.size();
    out.write(reinterpret_cast<const char*>(&numPoints), sizeof(numPoints));

    // Guardar los puntos
    for (const auto& point : points) {
        point.saveToFile(out, D);
    }

    // Guardar las rutas (paths)
    size_t numPaths = paths.size();
    out.write(reinterpret_cast<const char*>(&numPaths), sizeof(numPaths));
    for (const auto& p : paths) {
        size_t pathLength = p.size();
        out.write(reinterpret_cast<const char*>(&pathLength), sizeof(pathLength));
        out.write(p.c_str(), (long) pathLength);
    }
}




void SsInnerNode::loadFromStream(std::istream &in) {
    // Leer centroid
    centroid.readFromFile(in, D);

    // leer el valor del radio
    float radius_ = 0;
    in.read(reinterpret_cast<char*>(&radius_), sizeof(radius_));
    this->radius = radius_;

    // leer si apunta a hojas o nodos internos
    bool pointsToLeaf = false;
    in.read(reinterpret_cast<char*>(&pointsToLeaf), sizeof(pointsToLeaf));

    // leer cantidad de hijos
    size_t numChildren;
    in.read(reinterpret_cast<char*>(&numChildren), sizeof(numChildren));

    // leer hijos
    for (size_t i = 0; i < numChildren; ++i) {
        SsNode* child = pointsToLeaf ? static_cast<SsNode*>(new SsLeaf(D)) : static_cast<SsNode*>(new SsInnerNode(D));
        child->loadFromStream(in);
        children.push_back(child);
    }
}

void SsLeaf::loadFromStream(std::istream &in) {
    // Leer centroid
    centroid.readFromFile(in, D);

    // Leer radio
    float radius_ = 0;
    in.read(reinterpret_cast<char*>(&radius_), sizeof(radius_));
    this->radius = radius_;

    // Leer numero de puntos
    size_t numPoints;
    in.read(reinterpret_cast<char*>(&numPoints), sizeof(numPoints));

    // Leer puntos
    points.resize(numPoints);
    for (size_t i = 0; i < numPoints; ++i) {
        points[i].readFromFile(in, D);
    }

    // Leer rutas (paths)
    size_t numPaths;
    in.read(reinterpret_cast<char*>(&numPaths), sizeof(numPaths));
    paths.resize(numPaths);
    for (size_t i = 0; i < numPaths; ++i) {
        size_t pathLength;
        in.read(reinterpret_cast<char*>(&pathLength), sizeof(pathLength));
        char* buffer = new char[pathLength + 1];
        in.read(buffer, (long) pathLength);
        buffer[pathLength] = '\0';
        paths[i] = std::string(buffer);
        delete[] buffer;
    }
}

std::vector<Point> SsLeaf::getEntriesCentroids() const {
    return points;
}

void SsLeaf::sortEntriesByCoordinate(size_t coordinateIndex) {
    std::sort(points.begin(), points.end(), [coordinateIndex](const Point& a, const Point& b) {
        return a[coordinateIndex] < b[coordinateIndex];
    });
}

std::pair<SsNode *, SsNode *> SsLeaf::split() {
    size_t splitIndex = this->findSplitIndex();
    SsLeaf *newNode1 = new SsLeaf(D);
    SsLeaf *newNode2 = new SsLeaf(D);
    newNode1->points = {points.begin(), points.begin() + splitIndex };
    newNode2->points = {points.begin() + splitIndex, points.end()};



    newNode1->parent = this;
    newNode2->parent = this;

    newNode1->updateBoundingEnvelope();
    newNode2->updateBoundingEnvelope();


    return std::make_pair(newNode1, newNode2);
}

void SsLeaf::updateBoundingEnvelope() {
    std::vector<Point> pointss = this->getEntriesCentroids();

    std::pair<Point,NType> pp = findSmallestEnclosingBall(pointss);
    this->centroid = pp.first; this->radius = pp.second + 1.1;

}

std::pair<SsNode *, SsNode *> SsLeaf::insert(const Point &point, std::string path) {
    if(std::find(this->points.begin(), this->points.end(), point) != this->points.end()){
        return {nullptr, nullptr};
    }

    this->points.push_back(point);
    this->paths.push_back(path);
    this->updateBoundingEnvelope();

    if(this->points.size() <= Settings::M){
        return {nullptr, nullptr};
    }
    return this->split();
}

std::pair<SsNode *, SsNode *> SsLeaf::insert(const Point &point) {
    if(std::find(this->points.begin(), this->points.end(), point) != this->points.end()){
        return {nullptr, nullptr};
    }

    this->points.push_back(point);
    this->updateBoundingEnvelope();

    if(this->points.size() <= Settings::M){
        return {nullptr, nullptr};
    }
    return this->split();
}

void SsInnerNode::saveToStream(std::ostream &out) const {
    // Guardar centroid
    centroid.saveToFile(out, D);

    // Guardar el radio
    float radius_ = radius.getValue();
    out.write(reinterpret_cast<const char*>(&radius_), sizeof(radius_));

    // Guardar si apunta a nodos hoja
    bool pointsToLeafs = children[0]->isLeaf();
    out.write(reinterpret_cast<const char*>(&pointsToLeafs), sizeof(pointsToLeafs));

    // Guardar la cantidad de hijos para saber cuántos nodos leer después
    size_t numChildren = children.size();
    out.write(reinterpret_cast<const char*>(&numChildren), sizeof(numChildren));

    // Guardar los hijos
    for (const auto& child : children) {
        child->saveToStream(out);
    }
}

std::vector<Point> SsInnerNode::getEntriesCentroids() const {
    std::vector<Point> cent ;
    for(const auto& i : children){
        cent.push_back(i->centroid);
    }
    return cent;
}

void SsInnerNode::sortEntriesByCoordinate(size_t coordinateIndex) {
    std::sort(children.begin(), children.end(), [coordinateIndex](const SsNode* a, const SsNode* b) {
        return a->centroid[coordinateIndex] < b->centroid[coordinateIndex];
    });
}

std::pair<SsNode *, SsNode *> SsInnerNode::split() {
    size_t splitIndex = findSplitIndex();

    SsInnerNode* newNode1 = new SsInnerNode(D);
    SsInnerNode* newNode2 = new SsInnerNode(D);

    newNode1->children = {children.begin(), children.begin() + splitIndex };
    newNode2->children = {children.begin() + splitIndex, children.end()};

    newNode1->parent = this;
    newNode2->parent = this;

    newNode1->updateBoundingEnvelope();
    newNode2->updateBoundingEnvelope();


    return std::make_pair(newNode1, newNode2);

}

SsNode *SsInnerNode::findClosestChild(const Point &target) const {
    SsNode* r = nullptr;
    NType mindis = NType::max_value();
    for(SsNode* i : this->children){
        NType dis = distance(i->centroid,target);
        if(dis < mindis){
            mindis = distance(i->centroid,target);
            r = i;
        }
    }
    return r;
}

void SsInnerNode::updateBoundingEnvelope() {
    std::vector<Point> centroidss = this->getEntriesCentroids();
    this->centroid = centroidss[0];
    size_t k = centroidss[0].dim();
    for(size_t i = 0 ; i < k    ; i ++){
        NType su = 0;
        for(const Point& j : centroidss){
            su += j[i];
        }
        centroid[i] = su / (centroidss.size());
    }

    centroidss = this->findFarthestPointsInInnerNode(this->centroid);
    std::pair<Point,NType> pp = findSmallestEnclosingBall(centroidss);
    this->centroid = pp.first; this->radius = pp.second + 1.1;

}

std::vector<Point> SsInnerNode::findFarthestPointsInInnerNode(const Point& pp) {
    std::vector<Point> farthestPoints;

    for (const auto& child : children) {
        if (child->isLeaf()) {
            SsLeaf* leaf = dynamic_cast<SsLeaf*>(child);
            Point vec = leaf->findFarthestPointInLeaf(pp);
            farthestPoints.push_back(vec);
        } else {
            SsInnerNode* innerNode = dynamic_cast<SsInnerNode*>(child);
            std::vector<Point> childFarthestPoints = innerNode->findFarthestPointsInInnerNode(pp);
            for (const Point& p : childFarthestPoints) {
                farthestPoints.push_back(p);
            }
        }
    }

    return farthestPoints;
}

Point SsLeaf::findFarthestPointInLeaf(const Point &cc) {
    std::vector<Point> puntos = this->getEntriesCentroids();
    Point farthestPoint = puntos[0];
    NType maxDistance = distance(puntos[0], cc);

    for (size_t i = 1; i < puntos.size(); i++) {
        NType currentDistance = distance(puntos[i], cc);
        if (currentDistance > maxDistance) {
            maxDistance = currentDistance;
            farthestPoint = puntos[i];
        }
    }

    return farthestPoint;
}

std::pair<SsNode*,SsNode*> SsInnerNode::insert(const Point &point, std::string path) {
    SsNode* child = findClosestChild(point);

    std::pair<SsNode*,SsNode*> newChildren = child->insert(point) ;
    if( newChildren.first == nullptr){
        this->updateBoundingEnvelope();
        return std::make_pair(nullptr, nullptr);
    }
    this->children.erase(std::find(children.begin(), children.end(),child));
    this->children.push_back(newChildren.first);
    this->children.push_back(newChildren.second);
    this->updateBoundingEnvelope();
    if(this->children.size() <= Settings::M){
        return std::make_pair(nullptr, nullptr);
    }
    return this->split() ;

}


std::pair<SsNode*,SsNode*> SsInnerNode::insert(const Point &point) {
    SsNode* child = findClosestChild(point);

    std::pair<SsNode*,SsNode*> newChildren = child->insert(point) ;
    if( newChildren.first == nullptr){
        this->updateBoundingEnvelope();
        return std::make_pair(nullptr, nullptr);
    }
    this->children.erase(std::find(children.begin(), children.end(),child));
    this->children.push_back(newChildren.first);
    this->children.push_back(newChildren.second);
    this->updateBoundingEnvelope();
    if(this->children.size() <= Settings::M){
        return std::make_pair(nullptr, nullptr);
    }
    return this->split() ;
}


void SsTree::saveToFile(const std::string &filename) const {
    std::ofstream out(filename, std::ios::binary);
    if (!out) {
        throw std::runtime_error("Cannot open file for writing");
    }

    // Guardar las dimensiones de la estructura
    out.write(reinterpret_cast<const char*>(&D), sizeof(D));

    // Guardar si el root es hija o nodo interno
    bool isLeaf = root->isLeaf();
    out.write(reinterpret_cast<const char*>(&isLeaf), sizeof(isLeaf));
    //std::cout << root->radius << std::endl;
    // Guardar el resto de la estructura
    root->saveToStream(out);
    out.close();
}

void SsTree::loadFromFile(const std::string &filename) {
    std::ifstream in(filename, std::ios::binary);
    if (!in) {
        throw std::runtime_error("Cannot open file for reading");
    }
    if (root) {
        delete root;
        root = nullptr;
    }

    // Aquí se asume que el primer valor determina las dimensiones
    in.read(reinterpret_cast<char*>(&D), sizeof(D));

    // El segundo valor determina si el root es hoja
    bool isLeaf;
    in.read(reinterpret_cast<char*>(&isLeaf), sizeof(isLeaf));
    if (isLeaf) {
        root = new SsLeaf(D);
    } else {
        root = new SsInnerNode(D);
    }
    root->loadFromStream(in);
    in.close();
}


SsNode *SsTree::search(SsNode *node, const Point &target) {
    if(node->isLeaf()){
        SsLeaf* leaf = dynamic_cast<SsLeaf*>(node);
        for(const Point&i : leaf->points){
            if(i == target){
                return node;
            }
        }
        return nullptr;
    }else{
        SsInnerNode* innerNode = dynamic_cast<SsInnerNode*>(node);
        for(SsNode *i : innerNode->children){
            if(i->intersectsPoint(target)){
                SsNode* r = search(i,target);
                if(r != nullptr){
                    return r;
                }
            }
        }
        return nullptr;
    }
}

SsNode *SsTree::searchParentLeaf(SsNode *node, const Point &target) {
    if(node->isLeaf()){
        return node;
    }else{
        SsInnerNode* innerNode = dynamic_cast<SsInnerNode*>(node);
        SsNode* r =  innerNode->findClosestChild(target);
        return searchParentLeaf(r,target);
    }
}

void SsTree::insert(const Point &point) {
    if(this->root == nullptr){
        SsLeaf* temp = new SsLeaf(D);
        temp->points.push_back(point);
        temp->centroid = point;
        root = temp;
    }else {
        std::pair<SsNode *, SsNode *> result = this->root->insert(point);
        if (result.first != nullptr) {
            SsInnerNode *newRoot = new SsInnerNode(D);
            newRoot->children.push_back(result.first);
            newRoot->children.push_back(result.second);
            result.first = newRoot;
            result.second = newRoot;
            newRoot->updateBoundingEnvelope();
            root = newRoot;

        }
    }
}


void SsNode::DFirst(const Point &center, size_t k, std::vector<std::pair<NType, Point>> &result , NType& maxi)  {
    if(isLeaf()){
        const SsLeaf* temp = dynamic_cast<const SsLeaf*>(this);
        for(auto& item : temp->points){
            //std::cout << item << " ";
            NType pointDist = distance(center, item);
            if(pointDist <= maxi) {
                auto insertPos = std::upper_bound(result.begin(), result.end(), std::make_pair(pointDist, item),
                                                  [](const std::pair<NType, Point> &a,
                                                     const std::pair<NType, Point> &b) {
                                                      return a.first < b.first;
                                                  });
                result.insert(insertPos, std::make_pair(pointDist, item));
                if (result.size() > k) {
                    result.pop_back();
                    maxi = result.back().first ;
                }
            }
        }

    }else{
        const SsInnerNode* iiii = dynamic_cast<const SsInnerNode*>(this);
        for (auto& child : iiii->children) {
            NType childDist = distance(center, child->centroid);
            if (childDist <= maxi + child->radius) {
                child->DFirst(center, k, result, maxi);
            }
        }
    }

}


std::vector<Point> SsTree::kNNQuery(const Point& center, size_t k)  {
    std::vector<Point> result;
    std::vector<std::pair<NType ,Point>> knnResult;
    NType maxi = this->root->radius;
    root->DFirst(center, k, knnResult, maxi);

    for(const auto& pair : knnResult) {
        result.push_back(pair.second);
    }

    return result;
}

void SsTree::insert(const Point &point, const std::string &path) {
    if(this->root == nullptr){
        SsLeaf* temp = new SsLeaf(D);
        temp->points.push_back(point);
        temp->centroid = point;
        temp->paths.push_back(path);
        root = temp;

    }else {
        std::pair<SsNode *, SsNode *> result = this->root->insert(point,path);

        if (result.first != nullptr) {
            SsInnerNode *newRoot = new SsInnerNode(D);
            newRoot->children.push_back(result.first);
            newRoot->children.push_back(result.second);
            result.first = newRoot;
            result.second = newRoot;
            newRoot->updateBoundingEnvelope();
            root = newRoot;

        }
    }

}

