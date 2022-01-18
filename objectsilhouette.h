#ifndef OBJECTSILHOUETTE_H
#define OBJECTSILHOUETTE_H

#include <object.h>

class ObjectSilhouette : public Object
{
public:

    ObjectSilhouette();

    void Draw(mat4 &Translation, mat4 &mvp, mat4 &m, mat4 &v, vec3 &lightPos);

    void CreateSilhouette(vector<glm::vec3>, glm::vec3 color, int mode);

    void UpdateObject();    //An overload version of UpdateObject function

protected:

    int principalMode;

    bool ifInitialized;

    void GenBuffer();       //An overload version of GenBuffer function

    void GenObject();       //An overload version of GenObject function

    void ReceiveData(vector<glm::vec3>);
};

#endif // OBJECTSILHOUETTE_H
