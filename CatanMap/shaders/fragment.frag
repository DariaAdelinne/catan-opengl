#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec4 FragPosLightSpace;

uniform vec3 ourColor;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform float specularStrength;
uniform float shininess;
uniform vec3 emissive;
uniform float alpha;

uniform sampler2D shadowMap;

// Calculeaza cat de vizibil este fragmentul fata de lumina
float sampleShadow(vec3 normal, vec3 lightDir)
{
    // Transforma pozitia din spatiul luminii in coordonate de textura
    vec3 p = FragPosLightSpace.xyz / FragPosLightSpace.w;
    p = p * 0.5 + 0.5;

    // Fragmentele din afara shadow mapului raman luminate
    if (p.z > 1.0 || p.x < 0.0 || p.x > 1.0 || p.y < 0.0 || p.y > 1.0)
        return 1.0;

    float ndotl = max(dot(normal, lightDir), 0.0);

    // Biasul reduce artefactele de tip shadow acne
    float bias = max(0.0016 * (1.0 - ndotl), 0.00035);

    vec2 texel = 1.0 / textureSize(shadowMap, 0);

    // PCF pe 5x5 pentru margini de umbra mai netede
    float sum = 0.0;
    float weightSum = 0.0;
    for (int x = -2; x <= 2; x++)
    {
        for (int y = -2; y <= 2; y++)
        {
            float wx = 3.0 - abs(float(x));
            float wy = 3.0 - abs(float(y));
            float w = wx * wy;
            float closestDepth = texture(shadowMap, p.xy + vec2(x, y) * texel).r;

            // Compara adancimea curenta cu adancimea salvata in shadow map
            sum += (p.z - bias > closestDepth) ? 0.0 : w;
            weightSum += w;
        }
    }

    return sum / weightSum;
}

void main()
{
    // Normalizeaza normala primita de la vertex shader
    vec3 norm = normalize(Normal);

    // Directia luminii este folosita pentru iluminarea directa
    vec3 lightDir = normalize(lightPos);

    // Calculeaza cat de mult este orientata suprafata in sus
    float upness = clamp(norm.y * 0.5 + 0.5, 0.0, 1.0);

    vec3 skyLight    = vec3(0.92, 0.97, 1.05);
    vec3 groundLight = vec3(0.75, 0.72, 0.66);

    // Amesteca lumina de sol cu lumina de cer dupa orientarea normalei
    vec3 hemi = mix(groundLight, skyLight, upness);

    float ambientStrength = 0.58;

    // Lumina ambientala pastreaza obiectele vizibile si in zonele umbrite
    vec3 ambient = ambientStrength * ourColor * hemi;

    // Componenta difuza depinde de unghiul dintre normala si lumina
    float raw = max(dot(norm, lightDir), 0.0);
    float q   = raw;

    vec3 sunColor = vec3(1.05, 1.00, 0.90);
    vec3 diffuse  = q * sunColor * ourColor * 0.85;

    vec3 specular = vec3(0.0);

    // Calculeaza reflexia speculara doar pentru materialele care au specular
    if (specularStrength > 0.0 && raw > 0.0)
    {
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
        specular = specularStrength * spec * sunColor;
    }

    // Citeste vizibilitatea fragmentului in shadow map
    float shVis = sampleShadow(norm, lightDir);

    // Umbra afecteaza mai mult lumina directa decat ambientul
    float directShadow = mix(0.18, 1.0, shVis);
    float ambientShadow = mix(0.78, 1.0, shVis);

    // Ocluzie simpla bazata pe orientarea normalei
    float ao = clamp(0.88 + 0.12 * norm.y, 0.80, 1.0);

    // Combina lumina ambientala, difuza, speculara si emisiva
    vec3 color = (ambient * ambientShadow + diffuse * directShadow + specular * directShadow) * ao + emissive;

    // Tone mapping simplu pentru a controla intensitatea culorii
    color = color / (color + vec3(0.30));

    // Creste usor saturatia culorii finale
    float gray = dot(color, vec3(0.299, 0.587, 0.114));
    color = mix(vec3(gray), color, 1.3);

    // Corectie gamma aproximativa
    color = pow(max(color, vec3(0.0)), vec3(0.85));

    FragColor = vec4(clamp(color, 0.0, 1.0), alpha);
}