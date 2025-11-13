# ParticleSystem

```
ParticleSystem
`-- Group
    `-- Emitter
        `-- Particle
```

ParticleSystemはEditorなどを管理する

HashMap グループ名 : vector\<Emitter>

### Group
position : Vector3 - 基になる座標を持つ
emitters : vector\<Emitter> - Emitter群

### Emitter
position : Vector3 - Groupの座標から見た座標
particles : vector\<Particle> - Particle群

### Particle
position : Vector3 
velocity : Vector3
color : Vector4