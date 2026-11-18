#!/usr/bin/env python3
"""Génère Source/Presets.h

Chaque preset = reconstitution APPROXIMATIVE du style vocal d'un artiste
(archétype de chaîne + petits ajustements). Ce n'est pas une copie exacte.
Pour ajouter / modifier un artiste : éditer ARTISTS ci-dessous, puis lancer
    python tools/gen_presets.py
et recompiler.

Champs : comp, grain, bright (0-100), echo, space, width (0-100),
         hpf (Hz), mud (dB, <= 0), body (dB), deess (0-100)
"""
import os

ARCH = {
    "clean":   dict(comp=55, grain=15, bright=50, echo=8,  space=14, width=12, hpf=85,  mud=-3.0, body=1.5, deess=45),
    "melodic": dict(comp=62, grain=30, bright=55, echo=22, space=30, width=30, hpf=90,  mud=-3.0, body=0.5, deess=45),
    "trap":    dict(comp=65, grain=30, bright=60, echo=12, space=18, width=15, hpf=100, mud=-3.5, body=0.5, deess=50),
    "forward": dict(comp=66, grain=25, bright=55, echo=3,  space=6,  width=5,  hpf=90,  mud=-3.0, body=1.0, deess=50),
    "boombap": dict(comp=45, grain=30, bright=38, echo=3,  space=10, width=6,  hpf=75,  mud=-2.0, body=2.5, deess=40),
    "drill":   dict(comp=70, grain=35, bright=45, echo=4,  space=8,  width=6,  hpf=110, mud=-4.0, body=1.5, deess=50),
    "grime":   dict(comp=68, grain=30, bright=58, echo=4,  space=8,  width=6,  hpf=100, mud=-3.5, body=0.5, deess=50),
    "rnb":     dict(comp=45, grain=10, bright=68, echo=12, space=32, width=28, hpf=80,  mud=-2.5, body=1.0, deess=55),
    "pop":     dict(comp=55, grain=8,  bright=72, echo=10, space=25, width=25, hpf=90,  mud=-3.0, body=0.0, deess=60),
    "fr":      dict(comp=66, grain=22, bright=56, echo=8,  space=16, width=12, hpf=95,  mud=-3.5, body=1.0, deess=50),
    "frmel":   dict(comp=62, grain=22, bright=62, echo=14, space=26, width=22, hpf=90,  mud=-3.0, body=0.5, deess=52),
    "rage":    dict(comp=78, grain=65, bright=62, echo=15, space=18, width=20, hpf=120, mud=-4.5, body=0.0, deess=40),
    "latin":   dict(comp=60, grain=18, bright=62, echo=10, space=22, width=18, hpf=90,  mud=-3.0, body=0.5, deess=52),
    "afro":    dict(comp=52, grain=12, bright=60, echo=10, space=26, width=24, hpf=85,  mud=-2.5, body=1.0, deess=50),
    "emo":     dict(comp=62, grain=35, bright=44, echo=14, space=28, width=18, hpf=90,  mud=-3.0, body=1.5, deess=40),
    "poprap":  dict(comp=58, grain=20, bright=60, echo=10, space=20, width=20, hpf=85,  mud=-3.0, body=1.0, deess=50),
    "arab":    dict(comp=64, grain=20, bright=58, echo=8,  space=18, width=14, hpf=95,  mud=-3.5, body=1.0, deess=52),
}

# (catégorie, [(nom, archétype, {ajustements})...])
GENERIC = [
    ("Pro - Studio Clean",   dict(comp=55, grain=10, bright=55, echo=5,  space=12, width=10, hpf=80,  mud=-3.0, body=1.0, deess=50)),
    ("Pro - Radio Punch",    dict(comp=72, grain=25, bright=62, echo=4,  space=10, width=8,  hpf=100, mud=-3.5, body=1.0, deess=55)),
    ("Pro - Warm Analog",    dict(comp=50, grain=35, bright=40, echo=6,  space=16, width=12, hpf=75,  mud=-2.0, body=3.0, deess=40)),
    ("Pro - Air Pop",        dict(comp=55, grain=8,  bright=78, echo=10, space=26, width=24, hpf=90,  mud=-3.0, body=0.0, deess=60)),
    ("Pro - Dark Trap",      dict(comp=70, grain=35, bright=40, echo=10, space=22, width=16, hpf=110, mud=-4.0, body=2.0, deess=45)),
    ("Pro - Intimate Close", dict(comp=60, grain=12, bright=52, echo=2,  space=6,  width=4,  hpf=90,  mud=-3.0, body=2.5, deess=55)),
    ("Pro - Wide Stack",     dict(comp=60, grain=15, bright=60, echo=12, space=24, width=45, hpf=90,  mud=-3.0, body=0.5, deess=50)),
    ("Pro - Hard Rap",       dict(comp=78, grain=40, bright=60, echo=3,  space=6,  width=5,  hpf=100, mud=-4.0, body=1.0, deess=50)),
    ("Pro - Smooth RnB",     dict(comp=45, grain=10, bright=66, echo=10, space=30, width=26, hpf=80,  mud=-2.5, body=1.0, deess=55)),
    ("Pro - Lo-Fi Tape",     dict(comp=55, grain=60, bright=32, echo=12, space=22, width=14, hpf=120, mud=-3.0, body=1.0, deess=35)),
]

ARTISTS = [
    ("USA - Rap", [
        ("Drake", "clean", {}),
        ("J. Cole", "clean", dict(comp=-6, grain=+5, body=+0.5)),
        ("Kendrick Lamar", "forward", dict(bright=+3)),
        ("Eminem", "forward", dict(comp=+4, bright=+6, echo=-3, space=-3)),
        ("Jay-Z", "boombap", dict(bright=+6, comp=+5)),
        ("Nas", "boombap", dict(grain=-5, bright=+2)),
        ("The Notorious B.I.G.", "boombap", dict(bright=-6, body=+0.5)),
        ("2Pac", "boombap", dict(grain=+10, comp=+8, bright=+4)),
        ("Snoop Dogg", "boombap", dict(comp=-12, body=+1.0, space=+4)),
        ("50 Cent", "forward", dict(body=+1.0, grain=+5)),
        ("Kanye West", "forward", dict(grain=+10, space=+8, width=+8)),
        ("Lil Wayne", "trap", dict(grain=+10, bright=-2)),
        ("Future", "trap", dict(comp=+5, grain=+10, space=+5, echo=+4)),
        ("Travis Scott", "melodic", dict(comp=+10, grain=+10, echo=+5, space=+5, bright=-3)),
        ("Lil Baby", "trap", dict(comp=-5)),
        ("Gunna", "trap", dict(bright=+3, space=+4)),
        ("Young Thug", "trap", dict(grain=+5, width=+8, bright=+2)),
        ("Migos", "trap", dict(comp=+3, echo=+2)),
        ("Offset", "trap", dict(grain=+5)),
        ("Quavo", "melodic", dict(comp=-2, bright=+3)),
        ("21 Savage", "forward", dict(bright=-8, body=+1.5, space=-2)),
        ("Lil Uzi Vert", "rage", dict(comp=-10, grain=-15, bright=+3)),
        ("Playboi Carti", "rage", dict(grain=+5, width=+5)),
        ("Ken Carson", "rage", dict(grain=+5)),
        ("Yeat", "rage", dict(grain=-5, echo=-4, width=+5)),
        ("Juice WRLD", "emo", dict(comp=-2)),
        ("Lil Peep", "emo", dict(grain=+10, bright=-4)),
        ("XXXTentacion", "emo", dict(grain=+12, comp=+5, bright=-5)),
        ("Lil Tecca", "trap", dict(bright=+5, comp=-3)),
        ("Roddy Ricch", "trap", dict(bright=+3, space=+2)),
        ("Polo G", "emo", dict(bright=+2, comp=+3)),
        ("Lil Durk", "trap", dict(grain=+4)),
        ("NBA YoungBoy", "forward", dict(grain=+12, bright=-6, comp=+6)),
        ("DaBaby", "forward", dict(comp=+4, bright=+3)),
        ("Megan Thee Stallion", "forward", dict(bright=+4, space=+4)),
        ("Cardi B", "forward", dict(bright=+4, grain=+3)),
        ("Nicki Minaj", "poprap", dict(comp=+6)),
        ("Doja Cat", "poprap", dict(bright=+4, width=+5)),
        ("Post Malone", "poprap", dict(grain=+12, comp=+4, bright=-3)),
        ("Tyler, The Creator", "boombap", dict(grain=+10, bright=-3, body=+1.0)),
        ("A$AP Rocky", "melodic", dict(echo=-6, space=-4, grain=-5)),
        ("Kid Cudi", "emo", dict(echo=-4, bright=+4, space=+2)),
        ("Childish Gambino", "clean", dict(bright=+4, space=+6)),
        ("Pop Smoke", "drill", dict(grain=+6, body=+2.0, bright=-3)),
        ("Chief Keef", "drill", dict(grain=+8, comp=-5, bright=+2)),
        ("Fivio Foreign", "drill", dict(grain=+3, comp=+3)),
        ("Lil Nas X", "poprap", dict(bright=+5, width=+4)),
        ("Jack Harlow", "clean", dict(bright=+5, comp=+3)),
        ("Don Toliver", "melodic", dict(width=+6, grain=+3)),
        ("Lil Yachty", "trap", dict(bright=+5, width=+8, echo=+4)),
        ("Rod Wave", "emo", dict(bright=+4, comp=-5, space=+2)),
        ("Meek Mill", "forward", dict(comp=+3, grain=+8)),
        ("Rick Ross", "boombap", dict(body=+1.5, comp=-8, space=+3)),
        ("Ice Spice", "grime", dict(bright=+3, space=+2)),
        ("GloRilla", "forward", dict(comp=+6, bright=+2, grain=+8)),
        ("Latto", "forward", dict(bright=+4)),
    ]),
    ("UK - Drill / Grime", [
        ("Central Cee", "grime", dict(bright=+2, grain=-4)),
        ("Dave", "clean", dict(bright=+3, comp=+4)),
        ("Stormzy", "grime", dict(grain=+5, body=+1.0)),
        ("Skepta", "grime", dict(comp=+3, grain=+8)),
        ("Headie One", "drill", dict(bright=+3)),
        ("Digga D", "drill", dict(comp=+3, grain=+4)),
        ("Aitch", "grime", dict(bright=+2, space=+2)),
        ("J Hus", "afro", dict(comp=+8, grain=+6, bright=-3)),
        ("Little Simz", "clean", dict(bright=+5, grain=+4)),
        ("Ghetts", "grime", dict(comp=+4, grain=+6)),
        ("Dizzee Rascal", "grime", dict(grain=+8, bright=+4)),
        ("Slowthai", "grime", dict(grain=+12, comp=+4)),
        ("AJ Tracey", "grime", dict(bright=+3, space=+3)),
        ("Fredo", "grime", dict(comp=-4, bright=-2)),
    ]),
    ("France - Rap", [
        ("Booba", "fr", dict(grain=+8, body=+1.0, bright=-4, comp=+3)),
        ("SCH", "fr", dict(grain=+10, bright=-3, space=+4, comp=+4)),
        ("Ninho", "frmel", dict(comp=+3, bright=-2)),
        ("Jul", "frmel", dict(comp=+6, echo=+2, grain=+4)),
        ("PNL", "frmel", dict(echo=+6, space=+8, width=+6, grain=+3)),
        ("Gazo", "drill", dict(bright=+4, grain=-3)),
        ("Tiakola", "frmel", dict(bright=+3, width=+4)),
        ("Damso", "fr", dict(body=+3.0, bright=-6, grain=+4)),
        ("Orelsan", "forward", dict(echo=+1, comp=-3)),
        ("Nekfeu", "clean", dict(bright=+4, space=+2)),
        ("Lomepal", "clean", dict(grain=+6, bright=+2, space=+3)),
        ("Niska", "fr", dict(comp=+2, bright=+2)),
        ("Maes", "fr", dict(grain=+4, bright=-2)),
        ("Naps", "fr", dict(bright=+2, comp=+2)),
        ("Koba LaD", "fr", dict(comp=+4, grain=+5)),
        ("Hamza", "frmel", dict(width=+6, echo=+3)),
        ("Dinos", "fr", dict(comp=-3, space=+4)),
        ("Lacrim", "fr", dict(grain=+6, body=+1.0, bright=-3)),
        ("Kaaris", "fr", dict(grain=+10, comp=+6, bright=-2)),
        ("Vald", "fr", dict(grain=+8, bright=+3)),
        ("Freeze Corleone", "drill", dict(echo=+6, space=+10, bright=-3)),
        ("Zola", "frmel", dict(bright=+2, comp=+2)),
        ("Josman", "fr", dict(bright=+3, space=+2)),
        ("Dadju", "rnb", dict(bright=-2, comp=+6)),
        ("GIMS", "frmel", dict(bright=+6, space=+2)),
        ("Aya Nakamura", "frmel", dict(bright=+3, width=+4, echo=-2)),
        ("Werenoi", "fr", dict(comp=+3)),
        ("Leto", "fr", dict(grain=+4, comp=+2)),
        ("Gradur", "frmel", dict(echo=+4, bright=+2)),
        ("Heuss L'Enfoire", "fr", dict(grain=+6, bright=-1)),
        ("Fianso", "fr", dict(comp=+4, grain=+4)),
        ("Soprano", "pop", dict(comp=-2, bright=-4)),
    ]),
    ("Maroc / Maghreb / Arabe", [
        ("ElGrandeToto", "arab", dict(comp=+3, grain=+4)),
        ("Dizzy DROS", "arab", dict(bright=+3, space=+3)),
        ("Don Bigg", "arab", dict(grain=+4, bright=-3)),
        ("Draganov", "arab", dict(echo=+3, width=+4)),
        ("Muslim", "arab", dict(grain=+6, comp=+2)),
        ("Stormy", "arab", dict(bright=+2, space=+2)),
        ("Issam", "arab", dict(bright=+4, space=+6, comp=-4, grain=-6)),
        ("Saad Lamjarred", "pop", dict(bright=-2, comp=+3)),
        ("Soolking", "frmel", dict(bright=+3, echo=+2)),
        ("Balti", "arab", dict(grain=+2, space=+4)),
        ("Klay BBJ", "arab", dict(comp=+4, grain=+6)),
        ("Marwan Moussa", "arab", dict(grain=+8, bright=+3)),
        ("Wegz", "arab", dict(bright=+5, width=+5, echo=+3)),
        ("Marwan Pablo", "arab", dict(comp=+5, grain=+6, bright=-2)),
        ("Shabjdeed", "arab", dict(grain=+10, bright=-2)),
    ]),
    ("Latino", [
        ("Bad Bunny", "latin", dict(grain=+4, bright=-2)),
        ("Anuel AA", "latin", dict(comp=+6, grain=+6)),
        ("Myke Towers", "latin", dict(bright=+2)),
        ("Peso Pluma", "latin", dict(bright=-2, grain=+8, comp=+3)),
        ("Rauw Alejandro", "latin", dict(width=+6, bright=+3)),
        ("J Balvin", "latin", dict(bright=+4, comp=-2)),
        ("Ozuna", "latin", dict(echo=+3, space=+3)),
        ("Karol G", "pop", dict(width=+3, bright=+2)),
        ("Feid", "latin", dict(bright=+4, space=+3)),
        ("Quevedo", "latin", dict(comp=+3, bright=+2, grain=+4)),
        ("Duki", "latin", dict(grain=+8, comp=+4)),
        ("Morad", "latin", dict(grain=+6, comp=+6, bright=-3)),
    ]),
    ("Afro / Afrobeats", [
        ("Burna Boy", "afro", dict(grain=+6, body=+1.0)),
        ("Wizkid", "afro", dict(bright=+2, grain=-2)),
        ("Davido", "afro", dict(bright=+3, comp=+4)),
        ("Rema", "afro", dict(grain=+5, width=+4)),
        ("Asake", "afro", dict(comp=+8, grain=+4, bright=-2)),
        ("Tems", "rnb", dict(body=+2.0, bright=-4, space=+3)),
        ("Ayra Starr", "afro", dict(bright=+4, space=+3)),
        ("Fireboy DML", "afro", dict(bright=+2, width=+4)),
        ("Tyla", "pop", dict(bright=-2, width=+4)),
        ("Omah Lay", "afro", dict(echo=+4, space=+4)),
        ("CKay", "afro", dict(grain=+4, space=+4)),
    ]),
    ("RnB / Pop", [
        ("The Weeknd", "rnb", dict(echo=+6, space=+8, bright=+5, width=+4)),
        ("SZA", "rnb", dict(width=+4, bright=+2, space=+4)),
        ("Frank Ocean", "rnb", dict(grain=+6, bright=-4, comp=-4)),
        ("Bryson Tiller", "rnb", dict(comp=+4, bright=-3, echo=-3)),
        ("Usher", "rnb", dict(comp=+4, bright=+3)),
        ("Chris Brown", "rnb", dict(comp=+6, bright=+4)),
        ("Summer Walker", "rnb", dict(bright=-4, space=+4)),
        ("Daniel Caesar", "rnb", dict(grain=+6, comp=-6, bright=-4)),
        ("6LACK", "rnb", dict(echo=+3, bright=-6)),
        ("Brent Faiyaz", "rnb", dict(body=+1.5, bright=-4, comp=+4)),
        ("Beyonce", "pop", dict(comp=+6, bright=+3, width=+4)),
        ("Rihanna", "pop", dict(grain=+6, bright=-2, comp=+4)),
        ("Bruno Mars", "pop", dict(comp=+6, bright=+3, grain=+4)),
        ("Ariana Grande", "pop", dict(bright=+6, space=+4)),
        ("Dua Lipa", "pop", dict(body=+1.5, bright=-4, comp=+5)),
        ("Justin Bieber", "pop", dict(comp=+2, bright=+2)),
        ("Ed Sheeran", "pop", dict(body=+2.0, space=-6, bright=-6, grain=+6)),
        ("Billie Eilish", "pop", dict(comp=+10, bright=-6, grain=+8, space=-6, width=-4, body=+1.5)),
        ("Olivia Rodrigo", "pop", dict(grain=+10, bright=-2)),
    ]),
]

FIELDS = ["comp", "grain", "bright", "echo", "space", "width", "hpf", "mud", "body", "deess"]


def clamp(k, v):
    if k in ("comp", "grain", "bright", "echo", "space", "width", "deess"):
        return max(0.0, min(100.0, v))
    if k == "hpf":
        return max(40.0, min(200.0, v))
    if k == "mud":
        return max(-8.0, min(0.0, v))
    if k == "body":
        return max(-3.0, min(5.0, v))
    return v


def row(cat, name, d):
    vals = ", ".join(("%.1ff" % clamp(f, d[f])) for f in FIELDS)
    name_c = name.replace('"', '\\"')
    return '    {"%s", "%s", %s},' % (name_c, cat, vals)


def main():
    rows = []
    for name, d in GENERIC:
        rows.append(row("Pro - Generiques", name, d))
    for cat, items in ARTISTS:
        for name, arch, delta in items:
            d = dict(ARCH[arch])
            for k, v in delta.items():
                d[k] = d[k] + v
            rows.append(row(cat, name, d))
    out = os.path.join(os.path.dirname(__file__), "..", "Source", "Presets.h")
    with open(out, "w", encoding="utf-8") as f:
        f.write("// GENERE par tools/gen_presets.py - ne pas editer a la main\n")
        f.write("#pragma once\n\nnamespace sv\n{\n")
        f.write("struct PresetDef\n{\n    const char* name;\n    const char* category;\n")
        f.write("    float comp, grain, bright, echo, space, width, hpf, mud, body, deess;\n};\n\n")
        f.write("inline constexpr PresetDef kPresets[] = {\n")
        f.write("\n".join(rows))
        f.write("\n};\n\ninline constexpr int kNumPresets = (int) (sizeof (kPresets) / sizeof (kPresets[0]));\n}\n")
    print("OK :", len(rows), "presets")


if __name__ == "__main__":
    main()
