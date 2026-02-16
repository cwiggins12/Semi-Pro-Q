
#pragma once

#include <JuceHeader.h>
#include "CustomLookAndFeel.h"
#include "Utils/Constants.h"

//handles the consistent minimize/expand operations and the drag functions
struct MinimizableComponent : public juce::Component {
    MinimizableComponent(CustomLookAndFeelD& lnfd, int sx, int sy, int tlx, int tly) {
        minButton.setClickingTogglesState(true);
        minButton.setLookAndFeel(&lnfd);
        setSize(sx, sy);
        setTopLeftPosition(tlx, tly);
        size = { sx, sy };
        topLeft = { tlx, tly };
    }

    ~MinimizableComponent() override {
        minButton.setLookAndFeel(nullptr);
    }

    //minimizes child then calls a resize/repaint
    void applyMinimized(juce::Component& c, bool m) {
        isMin = m;
        if (m) {
            c.setTopLeftPosition(topLeft.x + size.x - MINIMIZE_BUTTON_DIM, topLeft.y);
            c.setSize(MINIMIZE_BUTTON_DIM, MINIMIZE_BUTTON_DIM);
        }
        else {
            c.setTopLeftPosition(topLeft);
            c.setSize(size.x, size.y);
        }
        c.resized();
        c.repaint();
    }

    //child can override and call to set their own buttons, must call within override with this namespace
    virtual void referValuesToButtons(juce::Value& v) {
        minButton.getToggleStateValue().referTo(v);
    }

    //get clicks to check for drags
    void mouseDown(const juce::MouseEvent& event) override {
        if (!(event.y > FADER_START_Y)) {
            dragger.startDraggingComponent(this, event);
        }
    }

    //if dragging, move and cache new topleft. Keeps within bounds of window
    virtual void mouseDrag(const juce::MouseEvent& event) override {
        dragger.dragComponent(this, event, nullptr);

        //drag and keep in comp bounds
        auto x = juce::jlimit(0, MAIN_WINDOW_WIDTH - getWidth(), getX());
        auto y = juce::jlimit(0, MAIN_WINDOW_HEIGHT - getHeight(), getY());

        //set comp topLeft
        setTopLeftPosition(x, y);
        topLeft.x = x;
        topLeft.y = y;
    }
    //called by child, checks minimized state
    bool isMin = false;
    //cached size and topLeft
    juce::Point<int> size, topLeft;
    //minimize button in top right of child
    juce::TextButton minButton;
    //drag functionality
    juce::ComponentDragger dragger;
};

//replaces all the juce labels in minimize comps. Way cheaper, easier to paint, and doesn't give issues with dragging
struct CheapLabel {
public:
    void setText(juce::String t) { text = t; }

    void setBounds(int x, int y, int w, int h) { bounds = { x, y, w, h }; }
    void setBounds(juce::Rectangle<int> b) { bounds = b; }

    //paints all but the outline
    void paint(Graphics& g) {
        g.setColour(juce::Colours::black);
        g.fillRect(bounds);

        g.setColour(juce::Colours::white);
        g.setFont(Font(14.0f, Font::plain));
        g.drawFittedText(text, bounds, Justification::centred, 1);
    }

    void paintOutline(Graphics& g) {
        g.setColour(juce::Colours::white);
        g.drawRect(bounds);
    }

    void paintAll(Graphics& g) {
        paint(g);
        paintOutline(g);
    }

private:
    juce::String text;
    juce::Rectangle<int> bounds;
};
