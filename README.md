# Pluria Design System

A reusable design system for designing and prototyping against the Pluria brand — across the **consumer mobile app**, the **Teams platform** (admin dashboard + self-sign-up), and the **marketing landing pages**.

> **Pluria — Workspaces Anywhere.** Choose where you work best: your HQ, a coworking space, or a cozy café. Pluria lets you book a desk or a meeting room in seconds. See where colleagues are headed and join with one tap — across 1,000+ workspaces in 15 countries in Latin America and Europe.

---

## Products in this system

| Product | Surface | Purpose |
|---|---|---|
| **Pluria App** (consumer) | iOS / Android | Find & book workspaces, see teammates' plans, check in, invite guests |
| **Pluria Teams** (platform) | Web dashboard | Admin self-sign-up, billing, member management, controls, insights |
| **Pluria Marketing** | Landing pages | Teams plan, Enterprise plan, Plans comparison page |

## Source material

All of the following were provided as inputs; they are the ground truth behind the tokens and components in this system.

- **Codebase** (read-only mount): `pluria-teams-platform-prototype/` — React 19 + Vite + Tailwind v4 + shadcn/ui + HugeIcons. Contains the Teams dashboard, mobile app, and landing page prototypes.
- **Brand tokens**: `uploads/colors_simplified.json` — latest color palette (canonical; overrides any older values).
- **Logo**: `uploads/pluria_logo.svg` → copied to `assets/pluria-logo-wordmark.svg`.
- **Display font**: `uploads/mollyo-medium.otf` → copied to `fonts/mollyo-medium.otf`. Used for handwritten marketing flourishes.
- **Brand Book**: `uploads/Brand Book Simplified.pdf` (not parsed into text; trust the codebase + color JSON as canonical).
- **Nexudus integration brief**: `uploads/[EN] Pluria - Nexudus Integration.pdf`.
- **Plans pages (screenshots)**: `uploads/Plans Page.png`, `uploads/Teams Plan.png`, `uploads/Enterprise Plan.png`.
- **Marketing examples (screenshots)**: `uploads/example_1.png` through `example_4.png` — agenda slide, "4 cosas" infographic, 2.5× LATAM growth slide, collaborative-work report.

---

## 📁 Index — what's in this folder

```
.
├── README.md                       (← you are here)
├── SKILL.md                        Agent Skill entry point (for Claude Code)
├── colors_and_type.css             All design tokens: color, type, spacing, radius, shadow, motion
│
├── assets/                         Brand assets — logos, illustrations, customer logos, GIFs
│   ├── pluria-logo.svg             Primary logo (from codebase — dark on light)
│   ├── pluria-logo-wordmark.svg    Wordmark (from brand upload)
│   ├── pluria-logo-teams.svg       "Pluria Teams" lockup for dashboard header
│   ├── google-icon.png             Used on auth screens
│   ├── hero.png                    Marketing hero photo
│   ├── hero-image.svg
│   ├── decorative-star.svg         Marketing sparkle
│   ├── decorative-wave.png         Marketing flourish
│   ├── coverage-map.svg            LATAM + Europe dotted map
│   ├── ill_landing_teams_top.png   Illustrations for marketing pages
│   ├── ill_landing_enterprise_top.png
│   ├── ill_enterprise_hq_*.png
│   ├── illustration_phone.png      "Continue in app" card
│   ├── illustration_insights_upgrade.png
│   ├── illustration_card_getting_started.png
│   ├── img_example_*.png           Marketing photography
│   ├── img_picture_add_*.png
│   ├── img_mobile_map_simulation.png
│   ├── m_onboarding_carousel.png
│   ├── export_*.gif                Animated product demos (controls, flexpoints, invites, approvals)
│   ├── check-icon.svg, flag-ro.png
│   └── customer_logos/             addi, deloitte, kueski, qubika, sdxworx, ui_path
│
├── fonts/
│   └── mollyo-medium.otf           Handwritten accent face
│
├── preview/                        Design System tab cards (one HTML per card)
│   ├── logo.html, colors-primary.html, colors-semantic.html, ...
│   └── components-*.html           Buttons, inputs, cards, badges, etc.
│
└── ui_kits/
    └── teams/                      Web dashboard UI kit
        ├── README.md
        ├── index.html              Interactive click-thru prototype
        ├── Sidebar.jsx             App sidebar w/ nav groups + enterprise upsell
        ├── Topbar.jsx
        ├── Card.jsx, Badge.jsx, Button.jsx
        ├── Input.jsx
        ├── AuthShell.jsx           Two-column auth layout
        └── screens/
            ├── LoginScreen.jsx
            ├── DashboardHome.jsx
            ├── PeopleScreen.jsx
            └── ActivityScreen.jsx
```

---

## 🎨 Visual Foundations

Pluria's visual language sits at the intersection of **playful warmth** (soft pale gradients, handwritten marker strokes, rounded everything) and **professional restraint** (generous whitespace, near-flat elevation, limited color range per screen). The product reads as *joyful but professional* — the exact tone the brand claims in voice.

### Color

- **Primary is green.** `--green-600` (#65B95A) is the CTA, the accent, the "active" state, the brand. Green carries the identity almost everywhere.
- **Yellow is the secondary highlight** — used sparingly as a marker-style underline (`.marker` class), a "new" badge background, or a background flourish behind hero numbers. Never a primary CTA.
- **Neutrals** are a cool slate scale (`--neutrals-500` = #7E9197, `--neutrals-900` = #0C2025). Text is always a deep charcoal, never pure black. The foreground is explicitly `#0C2025`.
- **Beige (`--beige-50`, #FCFBF6)** is used for the warm sidebar surface in the Teams dashboard. This warm cream is what differentiates Pluria from generic SaaS grays.
- **Accent scales** (coral, purple, teal, blue, orange) exist but are reserved — small role as chart colors or semantic illustration accents. Do not invent new palettes.
- **Gradients** are always **pale radial blobs**, not linear rainbows. See the "Enterprise upsell" card in the sidebar: three radial blobs (green-100, yellow-100, teal-100) drift slowly behind a white card. Landing banners use the same technique. No hard stops.

### Type

- **Poppins** for everything: UI, display, marketing. Weights 400 / 500 / 600 / 700. Letter-spacing tightens as size grows (`--tracking-tight: -0.02em` on headlines; `-0.03em` on h1).
- **Mollyo Medium** appears as a handwritten flourish on marketing slides (agendas, stat breakouts) — paired with a yellow marker underline. Never in UI.
- Body copy is **`--neutrals-500` / `--neutrals-600`** — muted, never full-contrast.
- Eyebrow labels are UPPERCASE, small, widely-tracked (`--tracking-widest`) — used above section headings and form titles.

### Spacing & Layout

- 4pt base grid (`--space-1` … `--space-24`).
- Dashboard uses an offcanvas sidebar + generous 48px content padding.
- Landing pages are vertical-rhythm heavy — each section breathes with ~96px of vertical space.
- Mobile app uses 16px gutters and 24px corner radii on all containers.

### Backgrounds

- Mostly **white** or `--neutrals-50`.
- **No full-bleed photography** behind product UI.
- Marketing uses **soft cream backgrounds** (beige-50 / green-50 tint) for section breaks.
- **Pale radial gradients** drift behind promotional cards (see sidebar upsell) — this is the signature "hero" effect.

### Corner radii

Everything is rounded. It's hard to overstate.
- Buttons: `--radius-md` (10px) or `--radius-lg` (14px).
- Cards: `--radius-xl` to `--radius-2xl` (18–24px).
- Hero banners and phone onboarding cards: `--radius-2xl` (24px).
- Pills / badges: `--radius-pill` (999px).
- Mobile app cards: 16–24px throughout.

### Shadows & elevation

- **Near-flat.** Pluria does not use dramatic drop shadows.
- Cards use `--shadow-card`: a 1px hairline + a very soft 12px blur at 4% opacity.
- Focus ring is a **green 3px ring** (`--shadow-ring`).
- No `inset` shadows. No neumorphism.

### Borders

- Hairline borders (`--neutrals-200` on light, `--neutrals-100` on very light surfaces).
- Sidebar has **transparent borders** — it's a visual container, not a boxed panel.
- Inputs get a 1px border + focus ring; validated states swap to green-500 (success) or red-400 (error) with a 3px ring.

### Animation

- **Easings:** primarily `--ease-out` (cubic-bezier(.22, 1, .36, 1)) for UI transitions; spring-ish (`--ease-spring`) for validation ✓/✗ toasts; `linear` for marquees.
- **Durations:** 120ms (fast), 220ms (standard), 360ms (slow).
- **Signature motion:** radial gradient drift on promotional cards (8–12s, `ease-in-out infinite`), avatar-jump on empty states, input shake on validation error, spring-in on tag validation.
- **No aggressive bounces.** No spinning loaders. Transitions are clean and calm.

### Hover / press states

- **Buttons:** darken one step (`--green-600 → --green-700`; `--neutrals-900 → --neutrals-800`). No scale, no shadow change.
- **Cards:** do not hover-transform in the dashboard. Marketing cards may lift slightly (`--shadow-card → --shadow-md`).
- **Press:** no shrink. Just the deeper background.

### Transparency / blur

- Used only on **map overlays** and **modal scrims** — always at low opacity.
- The sidebar upsell uses layered radial gradients at 40–60% opacity.
- No frosted/backdrop-blur glass. This is not an Apple-style system.

### Imagery

- **Warm, editorial photography** of real workspaces — cafés, coworkings, meeting rooms. Not corporate stock.
- Color grade is slightly warm, high-keyed, natural.
- Marketing illustrations use **flat color + minimal line** (see `ill_landing_teams_top.png`).
- No AI-generated or 3D-rendered imagery in the codebase.

---

## 🗣 Content Fundamentals

Pluria's voice is **joyful but professional** — warm, concise, second-person, emoji used with intent (not as decoration).

### Tone

- Direct, active verbs. "Invite your team." "Book a workspace." "Add billing info."
- First-person for user-centric microcopy ("**Welcome, Anna.**"), second-person for instructions ("Continue in the app").
- Occasional exclamation in celebration states ("**All set!**", "🎉 Code applied!"), never in error or neutral contexts.
- Short. Most marketing headlines are ≤ 6 words.

### Casing

- **Sentence case** for titles, headings, and buttons. ("Confirm email address", "Add billing information", "Upgrade to Enterprise")
- **UPPERCASE with wide tracking** for eyebrow labels only ("VERSION 2", "OR CONTINUE WITH").
- Never Title Case for buttons.

### Emoji usage

- **Yes** — but scarce and on-brand: 🎉 for success ("Code applied!"), 👋 for greetings ("Welcome to Pluria!"), 🇧🇷 / 🇨🇴 etc. for country flags in the location picker.
- **No** decorative emoji for bullet points, section markers, or card headers. The design does this with color + iconography.

### Copy examples (verbatim from product)

- Hero login: *"Welcome, Anna. Sales Team | 20 January 2026"*
- Setup checklist: *"Complete your account setup — 1 of 4 steps completed — Almost done!"*
- Empty state: *"Your account is fully configured. You're ready to go."*
- Marketing hero: *"One app for every workspace your team needs."*
- Nudge: *"Need help in completing account?"*
- FlexPoints: *"FlexPoints are your team's universal currency for workspace bookings. 1 FlexPoint ≈ €1."*
- Handwritten marketing slide: *"2.5× YoY growth in LATAM — More teams. More engagement. A better Pluria."*

### What to avoid

- ❌ Corporate jargon ("leverage", "synergy", "seamless").
- ❌ Marketing superlatives ("revolutionary", "world-class").
- ❌ Passive voice.
- ❌ Pronouns in buttons ("Your account" as a button label — use "Account").

### Multilingual

The product is shipped in **English, Spanish, and Portuguese** (EN / ES / PT). Localized copy files live at `prototypes/teams/i18n/{en,es,pt}.json` in the source repo. When generating new copy, preserve English as source-of-truth; ES/PT translations follow the same tone (warm, direct, sentence-case).

---

## 🧩 Iconography

Pluria uses **HugeIcons** as the primary icon system, specifically the **`@hugeicons/core-free-icons`** set rendered via the `@hugeicons/react` package, at `strokeWidth={1.5}` and `size={20}` (navigation) or `size={16}` (inline).

Usage guidelines:

- **Default stroke weight:** `1.5` — thin, elegant, never chunky.
- **Color:** `currentColor` so icons inherit from text color. Success checks use `#65B95A` directly.
- **Default size:** 20px in nav, 16px inline, 14–28px in promotional cards.
- **Filled vs stroke:** Pluria uses **stroke-style icons almost exclusively**. The one exception is `CheckmarkCircle01Icon` (filled) used for success confirmation.

### Icons in use (from `src/components/icons.tsx`)

`Mail01Icon`, `ArrowRight01Icon`, `ArrowLeft01Icon`, `CheckmarkCircle01Icon`, `UserGroupIcon`, `Globe02Icon`, `Tag01Icon`, `ArrowDown01Icon`, `LockPasswordIcon`, `Add01Icon`, `UserIcon`, `Home01Icon`, `Calendar01Icon`, `Analytics01Icon`, `Settings01Icon`, `CreditCardIcon`, `CustomerService01Icon`, `DashboardSquare01Icon`, `Building03Icon`, `Logout01Icon`, `Store01Icon`, `SmartPhone01Icon`, `CheckmarkSquare01Icon`, `Location01Icon`.

### For non-React artifacts

When building plain HTML (slides, one-off mocks), link HugeIcons from CDN:

```html
<link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/@hugeicons/font@latest/dist/hugeicons.css">
<!-- Or use Lucide as the closest substitute if the specific HugeIcons glyph isn't needed -->
```

Where a specific HugeIcon glyph isn't available via CDN, **Lucide** is the closest match (same 1.5 stroke weight, same geometric style) — document the substitution in any file that uses it.

### Other asset categories

- **Logo / wordmark:** `assets/pluria-logo.svg` (primary), `assets/pluria-logo-wordmark.svg` (from brand book), `assets/pluria-logo-teams.svg` (Teams product lockup).
- **Customer logos:** `assets/customer_logos/` — six customer wordmark PNGs used on landing pages (Addi, Deloitte, Kueski, Qubika, SD Worx, UiPath).
- **Illustrations:** Flat-color PNGs for marketing (`ill_landing_*.png`, `illustration_phone.png`, `illustration_insights_upgrade.png`).
- **Decorative marks:** `decorative-star.svg`, `decorative-wave.png` — used as small pop accents on marketing pages.
- **Animated explainers:** `export_approval_flow.gif`, `export_controls.gif`, `export_flexpoints.gif`, `export_invite_guests.gif` — use on feature callouts.
- **Emoji:** yes, but only the few listed above (🎉 👋 🇨🇴 🇧🇷 🇲🇽 …). Never for section markers.
- **Unicode symbols:** ● (active nav state), › (collapsed chevron), × (close), ✓ (inline check).

---

## 🧱 UI Kits

| Kit | Folder | What's inside |
|---|---|---|
| **Teams dashboard** | `ui_kits/teams/` | Interactive prototype of the admin dashboard — login → dashboard home → people → activity. Includes sidebar, topbar, cards, buttons, inputs, badges. |

See each kit's README for component list and usage.

---

## ⚠️ Caveats & substitutions

- **Mollyo Medium** is the provided display font (`fonts/mollyo-medium.otf`). It loads via `@font-face`. No substitution needed. If it ever fails to load, the fallback is `Caveat, Patrick Hand, cursive`.
- **HugeIcons** is linked from CDN for plain-HTML artifacts. For production React code, keep using `@hugeicons/react`.
- The **Brand Book PDF** (`uploads/Brand Book Simplified.pdf`) was not parsed into text. Color tokens were taken from `colors_simplified.json` (explicitly stated as the latest canonical source) and cross-verified against `src/index.css` in the codebase.
