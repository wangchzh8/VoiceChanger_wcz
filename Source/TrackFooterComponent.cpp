

 /*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2018
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com
*/

#include "Utilities.h"
#include "AudioClipComponent.h"
#include "EngineHelpers.h"
#include "PluginComponent.h"
#include "PluginMenu.h"
#include "PluginTreeGroup.h"

#include "TrackFooterComponent.h"

using namespace juce;
namespace te = tracktion_engine;

//==============================================================================
tracktion_engine::Plugin::Ptr showMenuAndCreatePlugin(tracktion_engine::Edit& edit)
{
    if (const auto tree = EngineHelpers::createPluginTree(edit.engine))
    {
        PluginTreeGroup root(edit, *tree, tracktion_engine::Plugin::Type::allPlugins);
        PluginMenu m(root);

        if (auto type = m.runMenu(root))
            return type->create(edit);
    }

    return {};
}



//==============================================================================
TrackFooterComponent::TrackFooterComponent(EditViewState& evs, te::Track::Ptr t)
    : editViewState(evs), track(t)
{
    addAndMakeVisible(addButton);

    buildPlugins();

    track->state.addListener(this);

    addButton.onClick = [this]
    {
        if (auto plugin = showMenuAndCreatePlugin(track->edit))
            track->pluginList.insertPlugin(plugin, 0, &editViewState.selectionManager);
    };
}

TrackFooterComponent::~TrackFooterComponent()
{
    track->state.removeListener(this);
}

void TrackFooterComponent::valueTreeChildAdded(juce::ValueTree&, juce::ValueTree& c)
{
    if (c.hasType(te::IDs::PLUGIN))
        markAndUpdate(updatePlugins);
}

void TrackFooterComponent::valueTreeChildRemoved(juce::ValueTree&, juce::ValueTree& c, int)
{
    if (c.hasType(te::IDs::PLUGIN))
        markAndUpdate(updatePlugins);
}

void TrackFooterComponent::valueTreeChildOrderChanged(juce::ValueTree&, int, int)
{
    markAndUpdate(updatePlugins);
}

void TrackFooterComponent::paint(Graphics& g)
{
    g.setColour(Colour(0xff202030));
    g.fillRect(getLocalBounds().withTrimmedLeft(2));

    if (editViewState.selectionManager.isSelected(track.get()))
    {
        g.setColour(Colours::red);
        g.drawRect(getLocalBounds().withTrimmedLeft(-4), 2);
    }
}

void TrackFooterComponent::mouseDown(const MouseEvent&)
{
    editViewState.selectionManager.selectOnly(track.get());
}

void TrackFooterComponent::resized()
{
    auto r = getLocalBounds().reduced(4);
    const int cx = 21;

    addButton.setBounds(r.removeFromLeft(cx).withSizeKeepingCentre(cx, cx));
    r.removeFromLeft(6);

    for (auto p : plugins)
    {
        p->setBounds(r.removeFromLeft(cx).withSizeKeepingCentre(cx, cx));
        r.removeFromLeft(2);
    }
}

void TrackFooterComponent::handleAsyncUpdate()
{
    if (compareAndReset(updatePlugins))
        buildPlugins();
}

void TrackFooterComponent::buildPlugins()
{
    plugins.clear();

    for (auto plugin : track->pluginList)
    {
        auto p = new PluginComponent(editViewState, plugin);
        addAndMakeVisible(p);
        plugins.add(p);
    }
    resized();
}
