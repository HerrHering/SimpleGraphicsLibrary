/**
 * @file SDFScene.hpp
 * @brief Dynamic GLSL source generation and macro injection for Raymarching primitives.
 *
 * @details
 * SHADER UNIFORM & LAYOUT EXPECTATIONS FOR RAYMARCHING FRAG SHADER:
 * - Uniforms required by generated fragment source:
 *     uniform vec2 u_Resolution; // Screen resolution (width, height)
 *     uniform mat4 u_InvView;    // Inverse Camera View Matrix (transforms screen rays to world space)
 *     uniform float u_Time;      // Elapsed time in seconds
 * - Macro string target in source text:
 *     #define DYNAMIC_SDF_CALL sdSphere(pObj, 1.0)
 * - Vertex attributes:
 *     layout (location = 0) in vec2 aPos;
 *     layout (location = 1) in vec2 aUV;
 *
 * @example Usage Example:
 * @code
 * #include "SDFScene.hpp"
 * #include "GraphicsManager.hpp"
 *
 * void updateScenePrimitive(GraphicsManager& gfx, SDFScene& scene) {
 *     // Switch scene SDF primitive state
 *     scene.currentPrimitive = PrimitiveType::TORUS;
 *
 *     // Re-generate shader string with swapped macro call and recompile
 *     gfx.createShaderFromSource(
 *         "Raymarch", 
 *         SDFScene::getDefaultVertexShaderSource(), 
 *         scene.getFragmentShaderSource()
 *     );
 * }
 * @endcode
 */

#pragma once
#include "ShaderLoader.hpp"
#include <string>

enum class PrimitiveType { SPHERE, BOX, TORUS };

class SDFScene {
public:
    PrimitiveType currentPrimitive = PrimitiveType::SPHERE;

    static std::string getDefaultVertexShaderSource() {
        return R"(
            #version 460 core
            layout (location = 0) in vec2 aPos;
            layout (location = 1) in vec2 aUV;

            out vec2 v_UV;

            void main() {
                v_UV = aUV; // Directly received from location 1 (0,0 Bottom-Left to 1,1 Top-Right)
                gl_Position = vec4(aPos, 0.0, 1.0);
            }
        )";
    }

    std::string getFragmentShaderSource() const {
        std::string rawSource = R"(
            #version 460 core
            out vec4 FragColor;
            in vec2 v_UV;

            uniform vec2 u_Resolution;
            uniform mat4 u_InvView;
            uniform float u_Time;

            float sdSphere(vec3 p, float r) { return length(p) - r; }
            float sdBox(vec3 p, vec3 b) { vec3 q = abs(p) - b; return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0); }
            float sdTorus(vec3 p, vec2 t) { vec2 q = vec2(length(p.xz)-t.x,p.y); return length(q)-t.y; }

            #define DYNAMIC_SDF_CALL sdSphere(pObj, 1.0)

            float map(vec3 p) {
                vec3 pObj = p - vec3(0.0, sin(u_Time * 2.0) * 0.5, 0.0);
                return DYNAMIC_SDF_CALL;
            }

            vec3 getNormal(vec3 p) {
                float d = map(p);
                vec2 e = vec2(0.001, 0.0);
                return normalize(vec3(
                    map(p + e.xyy) - map(p - e.xyy),
                    map(p + e.yxy) - map(p - e.yxy),
                    map(p + e.yyx) - map(p - e.yyx)
                ));
            }

            void main() {
                // v_UV goes from (0,0) [Bottom-Left] to (1,1) [Top-Right]
                // Centering UVs around origin (0,0): Y is positive UP, negative DOWN
                vec2 st = (v_UV - 0.5) * vec2(u_Resolution.x / u_Resolution.y, 1.0);
                
                vec3 rayDir = normalize(vec3(st, -1.0));
                rayDir = mat3(u_InvView) * rayDir;
                vec3 rayOrigin = u_InvView[3].xyz;

                float t = 0.0;
                for(int i = 0; i < 80; i++) {
                    vec3 p = rayOrigin + rayDir * t;
                    float d = map(p);
                    if (d < 0.001 || t > 50.0) break;
                    t += d;
                }

                vec3 col = vec3(0.05);
                if (t < 50.0) {
                    vec3 p = rayOrigin + rayDir * t;
                    vec3 n = getNormal(p);
                    vec3 lightDir = normalize(vec3(1.0, 2.0, 1.0));
                    float diff = max(dot(n, lightDir), 0.1);
                    col = vec3(0.2, 0.6, 0.9) * diff;
                }

                FragColor = vec4(col, 1.0);
            }
        )";

        std::string objCall;
        switch (currentPrimitive) {
            case PrimitiveType::SPHERE: objCall = "sdSphere(pObj, 1.0)"; break;
            case PrimitiveType::BOX:    objCall = "sdBox(pObj, vec3(0.8))"; break;
            case PrimitiveType::TORUS:  objCall = "sdTorus(pObj, vec2(0.8, 0.25))"; break;
        }

        return ShaderLoader::replaceMacro(rawSource, "#define DYNAMIC_SDF_CALL sdSphere(pObj, 1.0)", "#define DYNAMIC_SDF_CALL " + objCall);
    }
};