#version 330 core

in vec2 vUV;
out vec4 FragColor;

uniform sampler2D cardTex;
uniform vec3 backColor;
uniform int useTex;
uniform float alpha;

void main()
{
    // Muta coordonatele UV in jurul centrului cardului
    vec2 p = vUV - 0.5;

    // Raza folosita pentru colturile rotunjite
    float r = 0.05;

    // Calculeaza distanta fata de forma dreptunghiului rotunjit
    vec2 q = abs(p) - 0.5 + r;
    float d = min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - r;

    // Arunca pixelii din afara colturilor rotunjite
    if (d > 0.0)
        discard;

    vec4 col;

    // Foloseste textura cartii sau culoarea de fallback
    if (useTex == 1)
        col = texture(cardTex, vUV);
    else
        col = vec4(backColor, 1.0);

    // Aplica transparenta globala
    col.a *= alpha;

    FragColor = col;
}