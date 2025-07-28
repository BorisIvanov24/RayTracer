#include "Renderer.h"
#include <random>
#include <algorithm>
#include <fstream>
#include <iostream>
#include "CRTMaterial.h"

template <typename T>
T clamp(T value, T minVal, T maxVal) {
    return std::max(minVal, std::min(value, maxVal));
}

int scaleTo255(float value, float minVal, float maxVal) {
    float normalized = (value - minVal) / (maxVal - minVal);
    return static_cast<int>(clamp(normalized * 255.0f, 0.0f, 255.0f));
}

int floatToUint8(float value) {
    // Clamp value to [0.0f, 1.0f]
    value = std::max(0.0f, std::min(1.0f, value));
    return static_cast<int>(std::round(value * 255.0f));
}

// Function to convert triangle to RGB color
CRTVector triangleToColor(const CRTVector& p1, const CRTVector& p2, const CRTVector& p3) {
    // Compute centroid
    float cx = (p1.getX() + p2.getX() + p3.getX()) / 3.0f;
    float cy = (p1.getY() + p2.getY() + p3.getY()) / 3.0f;
    float cz = (p1.getZ() + p2.getZ() + p3.getZ()) / 3.0f;

    // Map centroid to RGB (assume coordinate bounds for normalization)
    int r = scaleTo255(cx, -1.0f, 1.0f); // adjust min/max bounds if needed
    int g = scaleTo255(cy, -1.0f, 1.0f);
    int b = scaleTo255(cz, -1.0f, 1.0f);

    return CRTVector(r, g, b);
}

CRTVector genRandomColor() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 255);
    return CRTVector(distrib(gen), distrib(gen), distrib(gen));
}


void writePixel(std::ofstream& ofs, const CRTVector& color) {
    int r = clamp(floatToUint8(color.getX()), 0, 255);
    int g = clamp(floatToUint8(color.getY()), 0, 255);
    int b = clamp(floatToUint8(color.getZ()), 0, 255);
    ofs << r << ' ' << g << ' ' << b << '\t';
}

CRTVector Renderer::calculatePointNormal(const CRTVector& point, const CRTMesh& mesh, int idx0, int idx1, int idx2) const
{
    CRTVector v0Normal = mesh.getVertexNormals()[idx0];
    CRTVector v1Normal = mesh.getVertexNormals()[idx1];
    CRTVector v2Normal = mesh.getVertexNormals()[idx2];

    CRTVector v0 = mesh.getVertices()[idx0];
    CRTVector v1 = mesh.getVertices()[idx1];
    CRTVector v2 = mesh.getVertices()[idx2];

    CRTVector v0v2 = v2 - v0;
    CRTVector v0v1 = v1 - v0;
    CRTVector v0P = point - v0;

    float u = cross(v0P, v0v2).length() / cross(v0v1, v0v2).length();
    float v = cross(v0v1, v0P).length() / cross(v0v1, v0v2).length();

    CRTVector pointNormal = v1Normal * u + v2Normal * v + v0Normal * (1 - u - v);

    return pointNormal;
}

CRTVector multiplyColors(const CRTVector& lhs, const CRTVector& rhs)
{
    return { lhs.getX() * rhs.getX(), lhs.getY() * rhs.getY(), lhs.getZ() * rhs.getZ() };
}

CRTVector Renderer::shade(const CRTRay& ray, const RayIntersectionData& data) const
{
    CRTVector resultColorVector(0.f, 0.f, 0.f);

    if (ray.getPathDepth() >= MAX_RAY_DEPTH || !data.isIntersected)
    {
        resultColorVector = scene->getSettings().backgroundColor;

        return resultColorVector;
    }

    const CRTMaterialType hitMaterialType = data.material->getType();

    if (hitMaterialType == CRTMaterialType::DIFFUSE)
    {
        resultColorVector = shadeDiffuse(ray, data);
    }
    else if (hitMaterialType == CRTMaterialType::REFLECTIVE)
    {
        resultColorVector = shadeReflective(ray, data);
    }
    else if (hitMaterialType == CRTMaterialType::REFRACTIVE)
    {
        resultColorVector = shadeRefractive(ray, data);
    }
    else if (hitMaterialType == CRTMaterialType::CONSTANT)
    {
        resultColorVector = shadeConstant(ray, data);
    }
    else
    {
        throw std::exception("Invalid material");
    }

    return resultColorVector;
}

CRTVector Renderer::shadeDiffuse(const CRTRay& ray, const RayIntersectionData& data) const
{
    const float shadowBias = 1e-2f;
    
    CRTVector finalColor(0.f, 0.f, 0.f);
    CRTVector albedo = data.material->getAlbedo();

    CRTVector normal = data.material->isSmoothShading() ? data.intersectionPointNormal : data.intersectionTriangle.getNormal();

    for (int i = 0; i < scene->getLights().size(); i++)
    {
        const CRTLight& light = scene->getLights()[i];

        CRTVector intersectionPoint = data.intersectionPoint;
        CRTVector lightPosition = light.getPosition();
        CRTTriangle intersectionTriangle = data.intersectionTriangle;

        CRTVector lightDir = lightPosition - intersectionPoint;
        float sphereRadius = lightDir.length();

        float maxT = lightDir.length() - 1e-2f;

        lightDir.normalise();

        float cosLaw = std::max(0.f, dot(lightDir, normal));
        float sphereArea = 4 * 3.14 * sphereRadius * sphereRadius;

        CRTRay shadowRay(intersectionPoint + intersectionTriangle.getNormal() * shadowBias, 
                         lightDir, ray.getPathDepth() + 1, CRTRayType::SHADOW);

        RayIntersectionData shadowRayData = traceRay(shadowRay, maxT);

        CRTVector lightContribution = (shadowRayData.isIntersected && 
                                      shadowRayData.material->getType() != CRTMaterialType::REFRACTIVE) ? CRTVector() :
                                      light.getIntensity() / sphereArea * albedo * cosLaw;

        finalColor = finalColor + lightContribution;
    }

    return finalColor;
}

CRTVector Renderer::shadeReflective(const CRTRay& ray, const RayIntersectionData& data) const
{
    CRTVector normal = data.intersectionPointNormal;

    CRTVector rayDir = ray.getDirection();
    rayDir.normalise();

    CRTVector reflectionRayDir = rayDir - 2 * dot(rayDir, normal) * normal;

    CRTRay reflectionRay(data.intersectionPoint + normal * 1e-2f, reflectionRayDir, 
                         ray.getPathDepth() + 1, CRTRayType::REFLECTION);

    RayIntersectionData reflectionRayData = traceRay(reflectionRay);

    CRTVector finalColor = shade(reflectionRay, reflectionRayData);

    finalColor = multiplyColors(finalColor, data.material->getAlbedo());

    return finalColor;
}

CRTVector Renderer::shadeRefractive(const CRTRay& ray, const RayIntersectionData& data) const
{
    float refractionBias = 1e-2f;
    float reflectionBias = 1e-2f;

    float nI = 1.f;
    float nR = data.material->getIor();

    CRTVector N = data.material->isSmoothShading() ? data.intersectionPointNormal : data.intersectionTriangle.getNormal();
    CRTVector I = ray.getDirection();

    if (dot(I, N) > 0)
    {
        N = N * -1.f;
        std::swap(nI, nR);
    }

    float cosAlpha = -dot(I, N);

    float sinAlpha = sqrtf(1.f - cosAlpha * cosAlpha);
    float ratio = nR / nI;

    if (sinAlpha > ratio)
    {
        //Total internal reflection
        CRTRay reflectedRay(data.intersectionPoint + (N * reflectionBias), I - 2.f * dot(I, N) * N,
            ray.getPathDepth() + 1, CRTRayType::REFLECTION);

        RayIntersectionData reflectedData = traceRay(reflectedRay);

        return shade(reflectedRay, reflectedData);
    }

    float sinBetha = (sinAlpha * nI) / nR;

    float cosBetha = sqrtf(1.f - sinBetha * sinBetha);

    CRTVector A = cosBetha * (N * -1.f);
    CRTVector C = (I + cosAlpha * N);
    C.normalise();
    CRTVector B = C * sinBetha;

    CRTVector R = A + B;

    CRTRay refractedRay(data.intersectionPoint + ((N * -1.f) * refractionBias), R, 
                        ray.getPathDepth() + 1, CRTRayType::REFRACTIVE);

    RayIntersectionData refractedData = traceRay(refractedRay);

    CRTVector refractedColor = shade(refractedRay, refractedData);


    CRTRay reflectedRay(data.intersectionPoint + (N * reflectionBias), I - 2.f * dot(I, N) * N,
                        ray.getPathDepth() + 1, CRTRayType::REFLECTION);

    RayIntersectionData reflectedData = traceRay(reflectedRay);

    CRTVector reflectedColor = shade(reflectedRay, reflectedData);


    float fresnel = 0.5f * (std::powf(1.f + dot(I, N), 5.f));

    return fresnel * reflectedColor + (1.f - fresnel) * refractedColor;

}

CRTVector Renderer::shadeConstant(const CRTRay& ray, const RayIntersectionData& data) const
{
    CRTVector finalColor = data.material->getAlbedo();

    return finalColor;
}

Renderer::Renderer(const CRTScene* scene) : scene(scene)
{

}

void Renderer::renderScene(const std::string& outputFile) const
{
    int screenWidth = scene->getSettings().imageWidth;
    int screenHeight = scene->getSettings().imageHeight;

    std::ofstream ofs(outputFile);
    ofs << "P3\n" << screenWidth << " " << screenHeight << "\n255\n";

    for (int j = 0; j < screenHeight; j++) {

        if (j % (screenHeight / 100) == 0)
            std::cout << (j / (screenHeight / 100)) << "%\n";

        for (int i = 0; i < screenWidth; i++) {

            CRTRay ray = genRay(i, j, scene->getCamera(), screenWidth, screenHeight);
            
            RayIntersectionData data = traceRay(ray);

            CRTVector finalColor = shade(ray, data);

            writePixel(ofs, finalColor);    
        }
        ofs << "\n";
    }
    std::cout << "100%\n";
    ofs.close();
}

void Renderer::renderAnimation(const std::string& outputFileBaseName) const
{
    int screenWidth = scene->getSettings().imageWidth;
    int screenHeight = scene->getSettings().imageHeight;

    for (int k = 0; k < 16; k++) 
    {
        CRTCamera camera = scene->getCamera();
        camera.panAroundTarget(k * 20, CRTVector(0.f, -5.f, 0.f));

        std::ofstream ofs(outputFileBaseName + std::to_string(k) + ".ppm");

        ofs << "P3\n" << screenWidth << " " << screenHeight << "\n255\n";

        for (int j = 0; j < screenHeight; j++) {

            if (j % (screenHeight / 100) == 0)
                std::cout << (j / (screenHeight / 100)) << "%\n";

            for (int i = 0; i < screenWidth; i++) {

                CRTRay ray = genRay(i, j, camera, screenWidth, screenHeight);

                RayIntersectionData data = traceRay(ray);

                CRTVector finalColor = shade(ray, data);

                writePixel(ofs, finalColor);
            }
            ofs << "\n";
        }
        std::cout << "100%\n";
        ofs.close();
        
    }
}

CRTRay Renderer::genRay(int x, int y, const CRTCamera& camera, int imageWidth, int imageHeight) const
{
    float xF = (x + 0.5f) / imageWidth;
    float yF = (y + 0.5f) / imageHeight;

    xF = 2.0f * xF - 1.0f;
    yF = 1.0f - 2.0f * yF;
    xF *= (float)imageWidth / imageHeight;

    CRTVector direction(xF, yF, -1.0f);
    direction = direction * camera.getRotationMatrix();
    direction.normalise();
    return CRTRay(camera.getPosition(), direction, 0, CRTRayType::CAMERA);
}

bool Renderer::isPointInTriangle(const CRTVector& point, const CRTTriangle& triangle) const
{
    CRTVector V0P = point - triangle.getVertex(0);
    CRTVector E0 = triangle.getVertex(1) - triangle.getVertex(0);

    //// First check: is the point in the triangle's plane?
    //float distance = dot(triangle.getNormal(), V0P);
    //if (fabs(distance) > 1e-4f)
    //    return false;

    if (dot(triangle.getNormal(), cross(E0, V0P)) < -0.00001f)
        return false;

    CRTVector V1P = point - triangle.getVertex(1);
    CRTVector E1 = triangle.getVertex(2) - triangle.getVertex(1);

    if (dot(triangle.getNormal(), cross(E1, V1P)) < -0.00001f)
        return false;

    CRTVector V2P = point - triangle.getVertex(2);
    CRTVector E2 = triangle.getVertex(0) - triangle.getVertex(2);

    if (dot(triangle.getNormal(), cross(E2, V2P)) < -0.00001f)
        return false;

    return true;
}

RayIntersectionData Renderer::traceRay(const CRTRay& ray, float maxT) const
{
    MinData minData;
    minData.t = -1.0f;
    
    for (size_t i = 0; i < scene->getObjects().size(); i++) {
        const auto& object = scene->getObjects()[i];
        const auto& vertices = object.getVertices();
        const auto& indices = object.getIndices();

        for (size_t j = 0; j + 2 < indices.size(); j += 3) {
            size_t idx0 = indices[j];
            size_t idx1 = indices[j + 1];
            size_t idx2 = indices[j + 2];

            // Bounds check
            if (idx0 >= vertices.size() || idx1 >= vertices.size() || idx2 >= vertices.size())
                continue;

            CRTVector v0 = vertices[idx0];
            CRTVector v1 = vertices[idx1];
            CRTVector v2 = vertices[idx2];

            // Degenerate triangle check
            if ((v1 - v0).length() < 1e-6f || (v2 - v1).length() < 1e-6f || (v0 - v2).length() < 1e-6f)
                continue;

            CRTTriangle triangle(v0, v1, v2);

            float rProj = dot(ray.getDirection(), triangle.getNormal());

            if (std::abs(rProj) < 0.0001f)
                continue;  // Parallel

            float rpDist = dot(triangle.getVertex(0) - ray.getOrigin(), triangle.getNormal());

            float t = rpDist / rProj;

            if (t < 0.f || t > maxT)
                continue;

            CRTVector p = ray.getOrigin() + t * ray.getDirection();

            if (isPointInTriangle(p, triangle)) {

                rpDist = std::abs(rpDist);
                if (minData.t < 0 || t < minData.t) {
                    minData.t = t;
                    minData.triangle = triangle;
                    minData.mesh = &object;
                    minData.idx0 = idx0;
                    minData.idx1 = idx1;
                    minData.idx2 = idx2;
                }
            }
        }
    }

    if (minData.t < 0)
    {
        return { false, CRTVector(), minData.triangle};
    }

    CRTVector intersectionPoint = ray.getOrigin() + minData.t * ray.getDirection();

    CRTVector pointNormal = calculatePointNormal(intersectionPoint, *minData.mesh,
                                                 minData.idx0, minData.idx1, minData.idx2);
    const CRTMaterial* material = &(scene->getMaterials()[minData.mesh->getMaterialIndex()]);

    RayIntersectionData toReturn;
    
    return { true, intersectionPoint, minData.triangle, pointNormal, material};
}

