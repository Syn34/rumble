// Scene geometry (src/engine/scene): one vertex format for every mesh, optional
// GPU skinning.
//
// Variants (defined by the build, see src/render/CMakeLists.txt):
//   RR_SKINNED  positions/normals/tangents are blended from up to four bone
//               matrices (DrawUniforms); `transform` then maps world space to clip
//               space and lightDir is in world space.
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec4 aTangent;   // w: bitangent sign
layout(location = 3) in vec2 aTexCoord;
layout(location = 4) in vec4 aColor;
#ifdef RR_SKINNED
layout(location = 5) in vec4 aBoneIndices;
layout(location = 6) in vec4 aBoneWeights;
#endif

RR_DRAW_CONSTANTS DrawConstants {
    mat4 transform;   // object (or, skinned, world) -> clip
    vec4 lightDir;    // xyz: towards the light in the same space; w: ambient level
    vec4 color;       // linear base colour and opacity
    vec4 uvTransform; // xy: scale, zw: offset of the main texture
    vec4 params;      // x: alpha cut-off (0 = off), y: normal-map strength,
                      // z: vertex colour weight, w: unused
} uDraw;

#ifdef RR_SKINNED
// Bone matrices: bone world transform * bind pose, in world space.
RR_DRAW_UNIFORMS DrawUniforms {
    mat4 bones[256];
} uSkin;
#endif

layout(location = 0) out vec3 vNormal;
layout(location = 1) out vec4 vTangent;
layout(location = 2) out vec2 vTexCoord;
layout(location = 3) out vec4 vColor;

void main() {
    vec4 position = vec4(aPosition, 1.0);
    vec3 normal = aNormal;
    vec3 tangent = aTangent.xyz;
#ifdef RR_SKINNED
    mat4 skin = uSkin.bones[int(aBoneIndices.x)] * aBoneWeights.x +
                uSkin.bones[int(aBoneIndices.y)] * aBoneWeights.y +
                uSkin.bones[int(aBoneIndices.z)] * aBoneWeights.z +
                uSkin.bones[int(aBoneIndices.w)] * aBoneWeights.w;
    position = skin * position;
    normal = mat3(skin) * normal;
    tangent = mat3(skin) * tangent;
#endif
    vNormal = normal;
    vTangent = vec4(tangent, aTangent.w);
    vTexCoord = aTexCoord * uDraw.uvTransform.xy + uDraw.uvTransform.zw;
    vColor = mix(vec4(1.0), aColor, uDraw.params.z);
    gl_Position = rr_ToClip(uDraw.transform * position);
}
