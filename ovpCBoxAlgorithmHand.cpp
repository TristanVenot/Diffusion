#include "ovpCBoxAlgorithmHand.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SignalProcessing;


#include <iostream>
#include <fstream>
#include <cstdlib>

#include <sys/timeb.h>

#include <tcptagging/IStimulusSender.h>

namespace OpenViBEPlugins {

	namespace SignalProcessing
	{
		gboolean flushing(gpointer pUserData)
		{
			reinterpret_cast<CBoxAlgorithmHand*>(pUserData)->flushQueue();

			return false;	// Only run once
		}


		//Size allocation of the window
		gboolean Hand_SizeAllocateCallback(GtkWidget *widget, GtkAllocation *allocation, gpointer data)
		{
			reinterpret_cast<CBoxAlgorithmHand*>(data)->resize((uint32)allocation->width, (uint32)allocation->height);
			return FALSE;
		}
		//Call for a drawing
		gboolean Hand_RedrawCallback(GtkWidget *widget, GdkEventExpose *event, gpointer data)
		{
			reinterpret_cast<CBoxAlgorithmHand*>(data)->redraw();
			
			return TRUE;
		}

		void CBoxAlgorithmHand::setStimulation(const uint32 ui32StimulationIndex, const uint64 ui64StimulationIdentifier, const uint64 ui64StimulationDate)
		{	
			boolean l_bStateUpdated = false;
			boolean left = false;
			boolean right = false;
			switch (ui64StimulationIdentifier)
			{
			case OVTK_GDF_End_Of_Trial:
				m_eCurrentState = EHandInterfaceState_Idle;
				l_bStateUpdated = true;
				
				break;

			case OVTK_GDF_End_Of_Session:
				m_eCurrentState = EHandInterfaceState_Idle;
				l_bStateUpdated = true;
				
				break;

			case OVTK_GDF_Cross_On_Screen:
				m_eCurrentState = EHandInterfaceState_Reference;
				l_bStateUpdated = true;
				
				break;

			case OVTK_GDF_Beep:
				// gdk_beep();
				getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Trace << "Beep is no more considered in 'Graz Visu', use the 'Sound player' for this!\n";
#if 0
#if defined TARGET_OS_Linux
				system("cat /local/ov_beep.wav > /dev/dsp &");
#endif
#endif
				break;





			case OVTK_GDF_Left:
				m_eCurrentState = EHandInterfaceState_Cue;
				m_eCurrentDirection = EArrowDirectionHand_Left;
				l_bStateUpdated = true;
				//test2 = true;
				this->getLogManager() << LogLevel_Warning << l_bStateUpdated;
				
				break;

			case OVTK_GDF_Right:
				m_eCurrentState = EHandInterfaceState_Cue;
				//test = true;
				m_eCurrentDirection = EArrowDirectionHand_Right;
				
				l_bStateUpdated = true;
				
				break;
			
			case OVTK_GDF_Up:
				m_eCurrentState = EHandInterfaceState_Cue;
				//test = true;
				m_eCurrentDirection = EArrowDirectionHand_Up;

				l_bStateUpdated = true;
				break;

			case OVTK_GDF_Down:
				m_eCurrentState = EHandInterfaceState_Cue;
				//test = true;
				m_eCurrentDirection = EArrowDirectionHand_Down;

				l_bStateUpdated = true;
				break;
			
			case OVTK_GDF_Feedback_Continuous:
				// New trial starts

				m_eCurrentState = EHandInterfaceState_ContinousFeedback;
				

				// as some trials may have artifacts and hence very high responses from e.g. LDA
				// its better to reset the max between trials
			
				l_bStateUpdated = true;
				
				break;
			}

			redraw();

				if (l_bStateUpdated)
			{
				processState();
			}



			// Queue the stimulation to be sent to TCP Tagging
			//m_vStimuliQueue.push_back(m_ui64LastStimulation);
		}

		void CBoxAlgorithmHand::processState()
		{
			switch (m_eCurrentState)
			{
			case EHandInterfaceState_Reference:
				if (GTK_WIDGET(m_pDrawingArea)->window)
					gdk_window_invalidate_rect(GTK_WIDGET(m_pDrawingArea)->window,
					NULL,
					true);
				break;

			case EHandInterfaceState_Cue:
				if (GTK_WIDGET(m_pDrawingArea)->window)
					gdk_window_invalidate_rect(GTK_WIDGET(m_pDrawingArea)->window,
					NULL,
					true);
				break;

			case EHandInterfaceState_Idle:
				if (GTK_WIDGET(m_pDrawingArea)->window)
					gdk_window_invalidate_rect(GTK_WIDGET(m_pDrawingArea)->window,
					NULL,
					true);
				break;

			case EHandInterfaceState_ContinousFeedback:
				if (GTK_WIDGET(m_pDrawingArea)->window)
					gdk_window_invalidate_rect(GTK_WIDGET(m_pDrawingArea)->window,
					NULL,
					true);
				break;

			default:
				break;
			}
		}

		//Construcor

		CBoxAlgorithmHand::CBoxAlgorithmHand(void) :
			m_pBuilderInterface(NULL),
			m_pMainWindow(NULL),
			m_eCurrentState(EHandInterfaceState_Idle),
			m_eCurrentDirection(EArrowDirectionHand_None),
			m_pDrawingArea(NULL),
			m_pActivityHandleft(NULL),
			m_pActivityHandleftmodified(NULL),
			m_pNeutralHandleft(NULL),
			m_pNeutralHandleftmodified(NULL),
			m_i64left(1),
			m_pActivityHandright(NULL),
			m_pActivityHandrightmodified(NULL),
			m_pNeutralHandright(NULL),
			m_pNeutralHandrightmodified(NULL),
			m_bFullScreen(false),
			test(false),
			test2(false),
			m_pStimulusSender(nullptr),
			m_bShowInstruction(true),
			m_visualizationContext(nullptr)
		
		{
			m_oBackgroundColor.pixel = 0;
			m_oBackgroundColor.red = 0;
			m_oBackgroundColor.green = 0;
			m_oBackgroundColor.blue = 0;

			m_oForegroundColor.pixel = 0;
			m_oForegroundColor.red = 0x8000;
			m_oForegroundColor.green = 0x8000;
			m_oForegroundColor.blue = 0x8000;
		}

		bool CBoxAlgorithmHand::initialize(void)
		{	
			m_bShowInstruction = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 0);
			m_bFullScreen = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 7);
			
			m_i64left = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 6);

			m_uiIdleFuncTag = 0;
			m_pStimulusSender = nullptr;
			

			m_vStimuliQueue.clear();

			m_oInput0Decoder.initialize(*this, 0);
			m_oInput1Decoder.initialize(*this, 1);


			//load the gtk builder interface
			
			m_pBuilderInterface = gtk_builder_new(); // glade_xml_new(OpenViBE::Directories::getDataDir() + "/plugins/simple-visualization/openvibe-simple-visualization-GrazVisualization.ui", NULL, NULL);
			gtk_builder_add_from_file(m_pBuilderInterface, OpenViBE::Directories::getDataDir() + "/plugins/signal-processing/openvibe-simple-visualization-Hand.ui", NULL);

			if (!m_pBuilderInterface)
			{
				this->getLogManager() << LogLevel_Error << "Error: couldn't load the interface!";
				return false;
			}

			gtk_builder_connect_signals(m_pBuilderInterface, NULL);
			
			m_pDrawingArea = GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "HandDrawingArea"));
			g_signal_connect(G_OBJECT(m_pDrawingArea), "expose_event", G_CALLBACK(Hand_RedrawCallback), this);
			g_signal_connect(G_OBJECT(m_pDrawingArea), "size-allocate", G_CALLBACK(Hand_SizeAllocateCallback), this);

			//set widget bg color
			gtk_widget_modify_bg(m_pDrawingArea, GTK_STATE_NORMAL, &m_oBackgroundColor);
			gtk_widget_modify_bg(m_pDrawingArea, GTK_STATE_PRELIGHT, &m_oBackgroundColor);
			gtk_widget_modify_bg(m_pDrawingArea, GTK_STATE_ACTIVE, &m_oBackgroundColor);

			gtk_widget_modify_fg(m_pDrawingArea, GTK_STATE_NORMAL, &m_oForegroundColor);
			gtk_widget_modify_fg(m_pDrawingArea, GTK_STATE_PRELIGHT, &m_oForegroundColor);
			gtk_widget_modify_fg(m_pDrawingArea, GTK_STATE_ACTIVE, &m_oForegroundColor);

			

			m_pActivityHandleft = gdk_pixbuf_new_from_file_at_size(OpenViBE::Directories::getDataDir() + "/plugins/signal-processing/Activityvirtual2.jpg", -1, -1, NULL);
			m_pActivityHandright = gdk_pixbuf_new_from_file_at_size(OpenViBE::Directories::getDataDir() + "/plugins/signal-processing/Activityvirtual2right.jpg", -1, -1, NULL);

			m_pNeutralHandleft = gdk_pixbuf_new_from_file_at_size(OpenViBE::Directories::getDataDir() + "/plugins/signal-processing/NeutralVirtual2left.png", -1, -1, NULL);
			m_pNeutralHandright = gdk_pixbuf_new_from_file_at_size(OpenViBE::Directories::getDataDir() + "/plugins/signal-processing/NeutralVirtual2.png", -1, -1, NULL);

			if (!m_pActivityHandleft || !m_pNeutralHandleft || !m_pActivityHandright || !m_pNeutralHandright)
			{
				this->getLogManager() << LogLevel_Error << "Error couldn't load arrow resource files!\n";

				return false;
			}

			m_pStimulusSender = TCPTagging::createStimulusSender();

			if (!m_pStimulusSender->connect("localhost", "15361"))
			{
				this->getLogManager() << LogLevel_Warning << "Unable to connect to AS's TCP Tagging plugin, stimuli wont be forwarded.\n";
			}

			if (m_i64left != 1 || m_i64left != 2)
			{
				this->getLogManager() << LogLevel_Warning << "Error : you must choose either right or left.\n";
			}

			if (m_bFullScreen)
			{
				GtkWidget *l_pWindow = gtk_widget_get_toplevel(m_pDrawingArea);
				gtk_window_fullscreen(GTK_WINDOW(l_pWindow));
				gtk_widget_show(l_pWindow);

				// @fixme small mem leak?
				GdkCursor* l_pCursor = gdk_cursor_new(GDK_BLANK_CURSOR);
				gdk_window_set_cursor(gtk_widget_get_window(l_pWindow), l_pCursor);
			}
			else
			{
				m_visualizationContext = dynamic_cast<OpenViBEVisualizationToolkit::IVisualizationContext*>(this->createPluginObject(OVP_ClassId_Plugin_VisualizationContext));
				m_visualizationContext->setWidget(*this, m_pDrawingArea);
			}

			// Invalidate the drawing area in order to get the image resize already called at this point. The actual run will be smoother.
			if (GTK_WIDGET(m_pDrawingArea)->window)
			{
				gdk_window_invalidate_rect(GTK_WIDGET(m_pDrawingArea)->window, NULL, true);
			}

			return true;
		}
		/*******************************************************************************/

		bool CBoxAlgorithmHand::uninitialize(void)
		{	
			// Remove the possibly dangling idle loop. 
			if (m_uiIdleFuncTag)
			{	
				m_vStimuliQueue.clear();
				g_source_remove(m_uiIdleFuncTag);
				m_uiIdleFuncTag = 0;
			}
			m_oInput0Decoder.uninitialize();
			m_oInput1Decoder.uninitialize();

			if (m_pStimulusSender)
			{
				delete m_pStimulusSender;
				m_pStimulusSender = nullptr;
			}
			

			// Close the full screen
			if (m_bFullScreen)
			{
				GtkWidget *l_pWindow = gtk_widget_get_toplevel(m_pDrawingArea);
				gtk_window_unfullscreen(GTK_WINDOW(l_pWindow));
				gtk_widget_destroy(l_pWindow);
			}

			//destroy drawing area
			if (m_pDrawingArea)
			{
				gtk_widget_destroy(m_pDrawingArea);
				m_pDrawingArea = nullptr;
			}

			// unref the xml file as it's not needed anymore
			
			g_object_unref(G_OBJECT(m_pBuilderInterface));
			m_pBuilderInterface = nullptr;
			



			if (m_pActivityHandleftmodified){ g_object_unref(G_OBJECT(m_pActivityHandleftmodified)); }
			if (m_pActivityHandleft){ g_object_unref(G_OBJECT(m_pActivityHandleft)); }

			if (m_pActivityHandrightmodified){ g_object_unref(G_OBJECT(m_pActivityHandrightmodified)); }
			if (m_pActivityHandright){ g_object_unref(G_OBJECT(m_pActivityHandright)); }

			if (m_pNeutralHandleftmodified){ g_object_unref(G_OBJECT(m_pNeutralHandleftmodified)); }
			if (m_pNeutralHandleft){ g_object_unref(G_OBJECT(m_pNeutralHandleft)); }

			if (m_pNeutralHandrightmodified){ g_object_unref(G_OBJECT(m_pNeutralHandrightmodified)); }
			if (m_pNeutralHandright){ g_object_unref(G_OBJECT(m_pNeutralHandright)); }




			return true;
		}
		/*******************************************************************************/

		bool CBoxAlgorithmHand::processInput(uint32 ui32InputIndex)
		{
			getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
			return true;
		}


		bool CBoxAlgorithmHand::process(void)
		{

			// the static box context describes the box inputs, outputs, settings structures
			const IBox& l_rStaticBoxContext = this->getStaticBoxContext();
			// the dynamic box context describes the current state of the box inputs and outputs (i.e. the chunks)
			IBoxIO& l_rDynamicBoxContext = this->getDynamicBoxContext();

			IBoxIO* l_pBoxIO = getBoxAlgorithmContext()->getDynamicBoxContext();

			// We decode and save the received stimulations.
			/*for (uint32 input = 0; input < getBoxAlgorithmContext()->getStaticBoxContext()->getInputCount(); input++)
			{
				for (uint32 chunk = 0; chunk < l_pBoxIO->getInputChunkCount(input); chunk++)
				{
					m_oInput0Decoder.decode(chunk, true);
					if (m_oInput0Decoder.isHeaderReceived())
					{
						// nop
					}
					if (m_oInput0Decoder.isBufferReceived())
					{
						for (uint32 stim = 0; stim < m_oInput0Decoder.getOutputStimulationSet()->getStimulationCount(); stim++)
						{
							// We always add the stimulations to the set to allow passing them to TCP Tagging in order in processClock()
							const uint64 l_ui64StimID = m_oInput0Decoder.getOutputStimulationSet()->getStimulationIdentifier(stim);
							const uint64 l_ui64StimDate = m_oInput0Decoder.getOutputStimulationSet()->getStimulationDate(stim);
							const uint64 l_ui64StimDuration = m_oInput0Decoder.getOutputStimulationSet()->getStimulationDuration(stim);

							const uint64 l_ui64Time = this->getPlayerContext().getCurrentTime();
							if (l_ui64StimDate < l_ui64Time)
							{
								const float64 l_f64Delay = ITimeArithmetics::timeToSeconds(l_ui64Time - l_ui64StimDate) * 1000; //delay in ms
								if (l_f64Delay>50)
									getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning << "Stimulation " << l_ui64StimID << " was received late: " << l_f64Delay << " ms \n";
							}

							if (l_ui64StimDate < l_pBoxIO->getInputChunkStartTime(input, chunk))
							{
								this->getLogManager() << LogLevel_ImportantWarning << "Input Stimulation Date before beginning of the buffer\n";
							}

							m_oPendingStimulationSet.appendStimulation(
								l_ui64StimID,
								l_ui64StimDate,
								l_ui64StimDuration);
						}
					}
				}
			}*/

			for (uint32 chunk = 0; chunk<l_pBoxIO->getInputChunkCount(0); chunk++)
			{
				m_oInput0Decoder.decode(chunk);
				if (m_oInput0Decoder.isBufferReceived())
				{
					const IStimulationSet* l_pStimulationSet = m_oInput0Decoder.getOutputStimulationSet();
					for (uint32 s = 0; s<l_pStimulationSet->getStimulationCount(); s++)
					{	
						
						setStimulation(s,
							l_pStimulationSet->getStimulationIdentifier(s),
							l_pStimulationSet->getStimulationDate(s));
					}
				}
			}


			if (m_uiIdleFuncTag == 0)
			{
				m_uiIdleFuncTag = g_idle_add(flushing, this);
			}



			// Tutorials:
			// http://openvibe.inria.fr/documentation/#Developer+Documentation
			// Codec Toolkit page :
			// http://openvibe.inria.fr/codec-toolkit-references/

			// Feel free to ask experienced developers on the forum (http://openvibe.inria.fr/forum) and IRC (#openvibe on irc.freenode.net).

			return true;
		}
		void CBoxAlgorithmHand::resize(uint32 ui32Width, uint32 ui32Height)
		{
		
			ui32Width = (ui32Width<8 ? 8 : ui32Width);
			ui32Height = (ui32Height<8 ? 8 : ui32Height);
		

			if (m_pActivityHandleftmodified)
			{
				g_object_unref(G_OBJECT(m_pActivityHandleftmodified));
			}

			m_pActivityHandleftmodified = gdk_pixbuf_scale_simple(m_pActivityHandleft, (2 * ui32Width) / 8, 6*ui32Height/8, GDK_INTERP_BILINEAR);

			if (m_pActivityHandrightmodified)
			{
				g_object_unref(G_OBJECT(m_pActivityHandrightmodified));
			}

			m_pActivityHandrightmodified = gdk_pixbuf_scale_simple(m_pActivityHandright, (2 * ui32Width) / 8, 6*ui32Height/8, GDK_INTERP_BILINEAR);



			if (m_pNeutralHandleftmodified)
			{
				g_object_unref(G_OBJECT(m_pNeutralHandleftmodified));
			}

			m_pNeutralHandleftmodified = gdk_pixbuf_scale_simple(m_pNeutralHandleft, (2 * ui32Width) / 8, 6*ui32Height/8, GDK_INTERP_BILINEAR);


			if (m_pNeutralHandrightmodified)
			{
				g_object_unref(G_OBJECT(m_pNeutralHandrightmodified));
			}

			m_pNeutralHandrightmodified = gdk_pixbuf_scale_simple(m_pNeutralHandright, (2 * ui32Width) / 8, 6*ui32Height/8, GDK_INTERP_BILINEAR);

		
		}

		void CBoxAlgorithmHand::redraw()
		{

			gint l_iWindowWidth = m_pDrawingArea->allocation.width;
			gint l_iWindowHeight = m_pDrawingArea->allocation.height;
			test = false;
			test2 = false;

			gint l_iX = 0;
			gint l_iY = 0;
			//increase line's width
			gdk_gc_set_line_attributes(m_pDrawingArea->style->fg_gc[GTK_WIDGET_STATE(m_pDrawingArea)], 1, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_BEVEL);

			switch (m_eCurrentState)
			{
			case EHandInterfaceState_Cue:
				drawTargets(m_bShowInstruction ? m_eCurrentDirection : EArrowDirectionHand_None);
				break;


			}

		}



	

		void CBoxAlgorithmHand::drawTargets(EArrowDirectionHand eDirection)
		{
			gint l_iWindowWidth = m_pDrawingArea->allocation.width;
			gint l_iWindowHeight = m_pDrawingArea->allocation.height;

			gint cpt = 0;
			gint l_iX = 0;
			gint l_iY = 0;

			//increase line's width
			gdk_gc_set_line_attributes(m_pDrawingArea->style->fg_gc[GTK_WIDGET_STATE(m_pDrawingArea)], 1, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_BEVEL);

			if (m_i64left == 1) {
				switch (eDirection)
				{
				case EArrowDirectionHand_None:
					//horizontal line
					gdk_draw_line(m_pDrawingArea->window,
						m_pDrawingArea->style->fg_gc[GTK_WIDGET_STATE(m_pDrawingArea)],
						(l_iWindowWidth / 4), (l_iWindowHeight / 2),
						((3 * l_iWindowWidth) / 4), (l_iWindowHeight / 2)
						);
					break;

				case EArrowDirectionHand_Left:
					l_iX = (l_iWindowWidth / 2) - (gdk_pixbuf_get_width(m_pActivityHandleftmodified)) + 300;
					l_iY = (l_iWindowHeight / 2) - (gdk_pixbuf_get_height(m_pActivityHandleftmodified) / 2);
					gdk_draw_pixbuf(m_pDrawingArea->window, NULL, m_pActivityHandleftmodified, 0, 0, l_iX, l_iY, -1, -1, GDK_RGB_DITHER_NONE, 0, 0);


					break;

				case EArrowDirectionHand_Right:
					l_iX = (l_iWindowWidth / 2) - (gdk_pixbuf_get_width(m_pNeutralHandleftmodified)) + 300;
					l_iY = (l_iWindowHeight / 2) - (gdk_pixbuf_get_height(m_pNeutralHandleftmodified) / 2);
					gdk_draw_pixbuf(m_pDrawingArea->window, NULL, m_pNeutralHandleftmodified, 0, 0, l_iX, l_iY, -1, -1, GDK_RGB_DITHER_NONE, 0, 0);


					break;
				}
			}
			if (m_i64left == 2) {
				switch (eDirection)
				{
				case EArrowDirectionHand_None:
					//horizontal line
					gdk_draw_line(m_pDrawingArea->window,
						m_pDrawingArea->style->fg_gc[GTK_WIDGET_STATE(m_pDrawingArea)],
						(l_iWindowWidth / 4), (l_iWindowHeight / 2),
						((3 * l_iWindowWidth) / 4), (l_iWindowHeight / 2)
						);
					break;
				case EArrowDirectionHand_Left:
					l_iX = (l_iWindowWidth / 2) - (gdk_pixbuf_get_width(m_pActivityHandrightmodified)) + 300;
					l_iY = (l_iWindowHeight / 2) - (gdk_pixbuf_get_height(m_pActivityHandrightmodified) / 2);
					gdk_draw_pixbuf(m_pDrawingArea->window, NULL, m_pActivityHandrightmodified, 0, 0, l_iX, l_iY, -1, -1, GDK_RGB_DITHER_NONE, 0, 0);


					break;

				case EArrowDirectionHand_Right:

					l_iX = (l_iWindowWidth / 2) - (gdk_pixbuf_get_width(m_pNeutralHandrightmodified)) + 300;
					l_iY = (l_iWindowHeight / 2) - (gdk_pixbuf_get_height(m_pNeutralHandrightmodified) / 2);
					gdk_draw_pixbuf(m_pDrawingArea->window, NULL, m_pNeutralHandrightmodified, 0, 0, l_iX, l_iY, -1, -1, GDK_RGB_DITHER_NONE, 0, 0);


					break;
				}
			}
		}
	




		void CBoxAlgorithmHand::drawReferenceCross()
		{
			gint l_iWindowWidth = m_pDrawingArea->allocation.width;
			gint l_iWindowHeight = m_pDrawingArea->allocation.height;

			//increase line's width
			gdk_gc_set_line_attributes(m_pDrawingArea->style->fg_gc[GTK_WIDGET_STATE(m_pDrawingArea)], 1, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_BEVEL);

			//horizontal line
			gdk_draw_line(m_pDrawingArea->window,
				m_pDrawingArea->style->fg_gc[GTK_WIDGET_STATE(m_pDrawingArea)],
				(l_iWindowWidth / 4), (l_iWindowHeight / 2),
				((3 * l_iWindowWidth) / 4), (l_iWindowHeight / 2)
				);
			//vertical line
			gdk_draw_line(m_pDrawingArea->window,
				m_pDrawingArea->style->fg_gc[GTK_WIDGET_STATE(m_pDrawingArea)],
				(l_iWindowWidth / 2), (l_iWindowHeight / 4),
				(l_iWindowWidth / 2), ((3 * l_iWindowHeight) / 4)
				);

			//increase line's width
			gdk_gc_set_line_attributes(m_pDrawingArea->style->fg_gc[GTK_WIDGET_STATE(m_pDrawingArea)], 1, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_BEVEL);

		}

		void CBoxAlgorithmHand::flushQueue(void)
		{
			for (size_t i = 0; i<m_vStimuliQueue.size(); i++)
			{
				m_pStimulusSender->sendStimulation(m_vStimuliQueue[i]);
			}
			m_vStimuliQueue.clear();

			// This function will be automatically removed after completion, so set to 0
			m_uiIdleFuncTag = 0;
		}


	};
};
