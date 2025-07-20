#include "Renderer.h"
#include <random>
#include <algorithm>
#include <fstream>
#include <iostream>

template <typename T>
T clamp(T value, T minVal, T maxVal) {
    return std::max(minVal, std::min(value, maxVal));
}

int scaleTo255(float value, float minVal, float maxVal) {
    float normalized = (value - minVal) / (maxVal - minVal);
    return static_cast<int>(clamp(normalized * 255.0f, 0.0f, 255.0f));
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
    int r = clamp((int)color.getX(), 0, 255);
    int g = clamp((int)color.getY(), 0, 255);
    int b = clamp((int)color.getZ(), 0, 255);
    ofs << r << ' ' << g << ' ' << b << '\t';
}

void Renderer::renderScene(const CRTScene& scene, const std::string& outputFile) {
    int screenWidth = scene.getSettings().imageWidth /2;
    int screenHeight = scene.getSettings().imageHeight /2;

    std::ofstream ofs(outputFile);
    ofs << "P3\n" << screenWidth << " " << screenHeight << "\n255\n";

    for (int j = 0; j < screenHeight; j++) {

        if (j % (screenHeight / 100) == 0)
            std::cout << (j / (screenHeight / 100)) << "%\n";

        for (int i = 0; i < screenWidth; i++) {
            
           /* if (i == 390 && j == 140) {
                writePixel(ofs, CRTVector(0, 0, 255));
                continue;
            }*/

            CRTRay ray = genRay(i, j, scene.getCamera(), screenWidth, screenHeight);
            CRTVector triColor(255, 0, 0);

            if (intersect(ray, scene, triColor, i, j)) {
                writePixel(ofs, triColor);

            } else {
                CRTVector backColor = scene.getSettings().backgroundColor * 255.0f;
                writePixel(ofs, backColor);
            }
        }
        ofs << "\n";
    }
    std::cout << "100%\n";
    ofs.close();
}

void Renderer::renderAnimation(const CRTScene& scene, const std::string& outputFile) {
    int screenWidth = scene.getSettings().imageWidth / 2;
    int screenHeight = scene.getSettings().imageHeight / 2;

    for (int k = 0; k < 6; k++) {
        CRTCamera camera = scene.getCamera();
        camera.panAroundTarget(k * 60, CRTVector(2.f, 2.f, -2.f));

        std::ofstream ofs(outputFile + static_cast<char>('0' + k) + ".ppm");
        ofs << "P3\n" << screenWidth << " " << screenHeight << "\n255\n";

        for (int j = 0; j < screenHeight; j++) {
            if (j % (screenHeight / 100) == 0)
                std::cout << (j / (screenHeight / 100)) << "%\n";

            for (int i = 0; i < screenWidth; i++) {

                CRTRay ray = genRay(i, j, camera, screenWidth, screenHeight);
                CRTVector triColor(255, 0, 0);

                if (intersect(ray, scene, triColor, i, j)) {
                    writePixel(ofs, triColor);
                } else {
                    CRTVector backColor = scene.getSettings().backgroundColor * 255.0f;
                    writePixel(ofs, backColor);
                }
            }
            ofs << "\n";
        }
        std::cout << "100%\n";
        ofs.close();
    }
}

CRTRay Renderer::genRay(int x, int y, const CRTCamera& camera, int imageWidth, int imageHeight) {
    float xF = (x + 0.5f) / imageWidth;
    float yF = (y + 0.5f) / imageHeight;

    xF = 2.0f * xF - 1.0f;
    yF = 1.0f - 2.0f * yF;
    xF *= (float)imageWidth / imageHeight;

    CRTVector direction(xF, yF, -1.0f);
    direction = direction * camera.getRotationMatrix();
    direction.normalise();
    return CRTRay(camera.getPosition(), direction);
}

bool Renderer::isPointInTriangle(const CRTVector& point, const CRTTriangle& triangle) {
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

bool Renderer::intersect(const CRTRay& ray, const CRTScene& scene, CRTVector& color, int k, int l) {
    float minRpDist = -1.0f;
    CRTVector finalColor;
    bool enableBackfaceCulling = true;
    CRTTriangle triangle1(CRTVector(-15.f, -5.f, 15.f), CRTVector(15.f, -5.f, 15.f), CRTVector(-15.f, -5.f, -15.f));
    CRTTriangle triangle2(CRTVector(15.f, -5.f, -15.f), CRTVector(-15.f, -5.f, -15.f), CRTVector(15.f, -5.f, 15.f));


    for (size_t i = 0; i < scene.getObjects().size(); i++) {
        const auto& object = scene.getObjects()[i];
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

            if (enableBackfaceCulling && rpDist > 0)
                continue;

            float t = rpDist / rProj;

            if (t < 0.f)
                continue;

            CRTVector p = ray.getOrigin() + t * ray.getDirection();

            if (isPointInTriangle(p, triangle)) {

                rpDist = std::abs(rpDist);
                if (minRpDist < 0 || t < minRpDist) {
                    minRpDist = t;

                    finalColor = CRTVector(i * 5 % 255, (j * 123123) % 255, 128); // unique color per triangle
                    //finalColor = triangleToColor(triangle.getVertex(0), triangle.getVertex(1), triangle.getVertex(2));
                    //finalColor = CRTVector(0, 0, 220);
                }
            }
        }
    }

    if (minRpDist < 0)
        return false;

    color = finalColor;

    return true;
}
