/*
 * This file is part of the DAWn distribution (https://github.com/GroovemanAndCo/DAWn).
 * Copyright (c) 2020 Fabien (https://github.com/fab672000)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once
#include <JuceHeader.h>
#include "ToolbarSlider.h"

 /**
  * Transport toolbar for the DAW main window
  */
class TransportToolbarItemFactory : public juce::ToolbarItemFactory
{
public:
    TransportToolbarItemFactory(juce::Button::Listener* listener, juce::Slider::Listener* sliderListener)
        : _toolbarButtonsListener(listener), _toolbarSliderListener(sliderListener) {}

    //==============================================================================
    // Each type of item a toolbar can contain must be given a unique ID. These
    // are the ones we'll use in this demo.
    enum TransportToolbarItemIds
    {
        doc_new = 1,
        doc_open = 2,
        doc_save = 3,
        doc_saveAs = 4,

        edit_copy = 11,
        edit_cut = 12,
        edit_paste = 13,

        media_start = 21,
        media_record = 22,
        media_stop = 23,
        media_pause = 24,
        media_forward = 25,
        media_backward = 26,

        tempo = 31
    };

    void getAllToolbarItemIds(juce::Array<int>& ids) override
    {
        // This returns the complete list of all item IDs that are allowed to
        // go in our toolbar. Any items you might want to add must be listed here. The
        // order in which they are listed will be used by the toolbar customisation panel.

        ids.add(doc_new);
        ids.add(doc_open);
        ids.add(doc_save);
        ids.add(doc_saveAs);

        ids.add(spacerId);

        //ids.add(media_backward );
        ids.add(media_stop);
        ids.add(media_start);
        ids.add(media_record);
        //ids.add(media_pause    );
        //ids.add(media_forward);

        //ids.add(edit_copy      );
        //ids.add(edit_cut       );
        //ids.add(edit_paste     );
        // If you're going to use separators, then they must also be added explicitly
        // to the list.
        //ids.add(separatorBarId);
        //ids.add(spacerId);
        //ids.add(flexibleSpacerId);
        ids.add(spacerId);
        ids.add(tempo);
    }

    void getDefaultItemSet(juce::Array<int>& ids) override
    {
        // This returns an ordered list of the set of items that make up a
        // toolbar's default set. Not all items need to be on this list, and
        // items can appear multiple times (e.g. the separators used here).
        ids.add(doc_new);
        ids.add(doc_open);
        ids.add(doc_save);
        ids.add(doc_saveAs);

        ids.add(spacerId);

        //ids.add(media_backward );
        ids.add(media_stop);
        ids.add(media_start);
        ids.add(media_record);
        //ids.add(media_pause    );
        //ids.add(media_forward);
        ids.add(spacerId);
        ids.add(tempo);
    }

    juce::ToolbarItemComponent* createItem(int itemId) override
    {
        switch (itemId)
        {
        case doc_new:           return createButtonFromZipFileSVG(itemId, "new", "document-new.svg");
        case doc_open:          return createButtonFromZipFileSVG(itemId, "open", "document-open.svg");
        case doc_save:          return createButtonFromZipFileSVG(itemId, "save", "document-save.svg");
        case doc_saveAs:        return createButtonFromZipFileSVG(itemId, "save as", "document-save-as.svg");

        case edit_copy:         return createButtonFromZipFileSVG(itemId, "copy", "edit-copy.svg");
        case edit_cut:          return createButtonFromZipFileSVG(itemId, "cut", "edit-cut.svg");
        case edit_paste:        return createButtonFromZipFileSVG(itemId, "paste", "edit-paste.svg");

        case media_start:       return createButtonFromZipFileSVG(itemId, "start", "media-playback-start.svg");
        case media_record:      return createButtonFromZipFileSVG(itemId, "record", "media-record.svg");
        case media_stop:        return createButtonFromZipFileSVG(itemId, "stop", "media-playback-stop.svg");
        case media_pause:       return createButtonFromZipFileSVG(itemId, "pause", "media-playback-pause.svg");
        case media_forward:     return createButtonFromZipFileSVG(itemId, "forward", "media-seek-forward.svg");
        case media_backward:    return createButtonFromZipFileSVG(itemId, "backward", "media-seek-backward.svg");
        case tempo:             return createTempoSlider(itemId);
        default:                break;
        }

        return nullptr;
    }

    [[nodiscard]] juce::Slider* getTempoSlider() const {}
    inline const static juce::String TempoSliderName = "Global Tempo";


private:
    juce::StringArray _iconNames;
    juce::OwnedArray<juce::Drawable> _iconsFromZipFile;
    juce::Button::Listener* _toolbarButtonsListener;
    juce::Slider::Listener* _toolbarSliderListener;
    juce::Slider* _slider;

    // This is a little utility to create a button with one of the SVG images in
    // our embedded ZIP file "res_icons_zip"
    juce::ToolbarItemComponent* createTempoSlider(const int itemId)
    {
        _slider = new juce::Slider(); // weak ptr
        _slider->setName(TempoSliderName);
        _slider->setSliderStyle(juce::Slider::SliderStyle::LinearBar);
        _slider->setRange(10, 250, 0.001);
        _slider->setValue(120.0, juce::NotificationType::dontSendNotification);
        _slider->addListener(_toolbarSliderListener);
        auto* toolbarSlider = new ToolbarSlider(_slider, itemId, 60, 40, 80);

        return toolbarSlider;
    }

    // This is a little utility to create a button with one of the SVG images in
    // our embedded ZIP file "res_icons_zip"
    juce::ToolbarItemComponent* createButtonFromZipFileSVG(const int itemId, const juce::String& text, const juce::String& filename)
    {
        if (_iconsFromZipFile.size() == 0)
        {
            // If we've not already done so, load all the images from the zip file..
            juce::ZipFile icons(Helpers::createZipStreamFromEmbeddedResource("res_icons_zip").release(), true);
            // DBG(icons.getNumEntries()); // why equals 0?
            for (int i = 0; i < icons.getNumEntries(); ++i)
            {
                std::unique_ptr<juce::InputStream> svgFileStream(icons.createStreamForEntry(i));

                if (svgFileStream.get() != nullptr)
                {
                    _iconNames.add(icons.getEntry(i)->filename);
                    _iconsFromZipFile.add(juce::Drawable::createFromImageDataStream(*svgFileStream));
                }
            }
        }

        auto* image = _iconsFromZipFile[_iconNames.indexOf(filename)];
        auto* button = new juce::ToolbarButton(itemId, text, image->createCopy(), {});
        button->addListener(_toolbarButtonsListener);

        return button;
    }
};

