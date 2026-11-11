# Portfolio Guide

## Files

```
portfolio/
├── index.html                  ← main page (edit this the most)
├── style.css                   ← colors, fonts, layout (change --accent at the top to restyle)
├── resume.pdf                  ← ADD THIS: export your résumé as PDF with exactly this name
├── images/                     ← put all photos here
│   └── placeholder.svg
└── projects/
    └── project-template.html   ← copy this for each project write-up
```

To preview, double-click `index.html` to open it in your browser. Refresh after every save.
Every spot you need to change is marked with `EDIT:` in a comment. In VS Code, press Cmd+Shift+F and search `EDIT:`.

---

## Step 1: Basics (about 30 minutes)
- [ ] Replace "Your Name", "YN" initials, school, and grad year (search `Your Name` and `YN`)
- [ ] Update GitHub, LinkedIn, and email links
- [ ] Add `resume.pdf` to the folder
- [ ] Write the hero sentence: what you're interested in, and which internships you're looking for

## Step 2: Projects (the most important part)

**What counts as a project?** More than you think:
- Personal builds (Arduino, Raspberry Pi, guitar pedal, LED cube, power supply)
- Course labs and design projects, especially open-ended ones
- Club / competition team work (robotics, solar car, Formula SAE, IEEE)
- Hackathon projects
- Even a well-documented failed project. Debugging stories make great interview material.

Aim for **3–4 projects**. Two good write-ups beat six thin ones. Put your strongest project first.

**To add a project card:** in `index.html`, copy the block between
`PROJECT CARD: copy everything from here...` and `...to here`, paste it, and edit it.

**To add a full write-up page:**
1. Duplicate `projects/project-template.html` and rename it, e.g. `projects/line-follower.html`
2. Fill in the sections (Overview, My role, Design, Challenges, Results, What I'd do differently)
3. Point the card's "Read more →" link at it: `href="projects/line-follower.html"`

**Writing tips for EE recruiters:**
- Lead with what it *does*, then how you built it.
- Use specific part names (ATmega328P, LM317, MPU-6050). It shows you actually did the work.
- Include numbers wherever you have them: voltages, current draw, sample rates, cost, accuracy.
- Spend the most space on the "Challenges & debugging" section. Recruiters ask about this in interviews.
- For team projects, be explicit about your part: "I designed the power distribution board."

## Step 3: Pictures

**What to photograph for each project:**
1. The finished thing, cleanly lit (use this on the card)
2. Work in progress: breadboard prototype, soldering, enclosure build
3. **Schematic**, exported from KiCad, Eagle, or LTspice as PNG. Recruiters look for this.
4. **PCB layout or 3D render**, if you made a board
5. **Oscilloscope screenshot or data plot** showing it works
6. Block diagram (draw.io or Excalidraw, both free)

**How to add them:**
- Put the file in `images/`. Use lowercase names with no spaces: `line-follower-front.jpg`
- From `index.html`: `<img src="images/line-follower-front.jpg" alt="Line follower robot">`
- From a page in `projects/`: `<img src="../images/line-follower-front.jpg" ...>` (note the `../`)
- **Resize before adding.** Phone photos are 3–5 MB. Shrink them to about 1600px wide
  (on a Mac, open in Preview → Tools → Adjust Size) so the site loads fast.
- Card images are cropped to a 16:10 shape, so center your subject.
- Photo tips: plain background, daylight from a window, clean up stray wires.

**Videos:** upload to YouTube as Unlisted, then use the commented-out `<iframe>` in the template.

## Step 4: Skills, experience, coursework
- **Skills:** only list things you could answer a question about. Edit the tags in each group.
- **Experience:** clubs, research, TA/tutoring, and part-time jobs all count. Start bullets with verbs.
- **Coursework:** list upper-level or relevant courses. Add new ones each semester.

## Step 5: Put it online (free) with GitHub Pages
1. Create a GitHub account if you don't have one.
2. Create a new **public** repository named exactly `yourusername.github.io`.
3. Upload all the files in this folder: "Add file → Upload files", then drag everything in, keeping the folders.
4. Go to Settings → Pages. Under "Branch", choose `main` and `/ (root)`, then Save.
5. After about a minute, your site is live at `https://yourusername.github.io`.
6. Put that link on your résumé, LinkedIn, and Handshake profile.

To update the site later, upload the changed files again (or learn `git push`, which is a good skill to pick up).

## Keep it fresh
- Update it at the **end of each semester** with new projects and courses.
- Before applying, check that every link works and test the site on your phone.
- Remove placeholder text completely. A leftover "Project Title" looks worse than a missing project.
