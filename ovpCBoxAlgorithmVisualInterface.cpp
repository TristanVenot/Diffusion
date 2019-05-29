#include "ovpCBoxAlgorithmVisualInterface.h"

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
		gboolean flush(gpointer pUserData)
		{
			reinterpret_cast<CBoxAlgorithmVisualInterface*>(pUserData)->flushQueue();

			return false;	// Only run once
		}


		//Size allocation of the window
		gboolean VisualInterface_SizeAllocateCallback(GtkWidget *widget, GtkAllocation *allocation, gpointer data)
		{
			reinterpret_cast<CBoxAlgorithmVisualInterface*>(data)->resize((uint32)allocation->width, (uint32)allocation->height);
			return FALSE;
		}
		//Call for a drawing
		gboolean VisualInterface_RedrawCallback(GtkWidget *widget, GdkEventExpose *event, gpointer data)
		{
			reinterpret_cast<CBoxAlgorithmVisualInterface*>(data)->redraw();
			return TRUE;
		}

		void CBoxAlgorithmVisualInterface::setStimulation(const uint32 ui32StimulationIndex, const uint64 ui64StimulationIdentifier, const uint64 ui64StimulationDate)
		{
			boolean l_bStateUpdated = false;

			switch (ui64StimulationIdentifier)
			{
			case OVTK_GDF_End_Of_Trial:
				m_eCurrentState = EVisualInterfaceState_Idle;
				l_bStateUpdated = true;

				break;

			case OVTK_GDF_End_Of_Session:
				m_eCurrentState = EVisualInterfaceState_Idle;
				l_bStateUpdated = true;

				break;

			case OVTK_GDF_Cross_On_Screen:
				m_eCurrentState = EVisualInterfaceState_Reference;
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
				m_eCurrentState = EVisualInterfaceState_Cue;
				m_eCurrentDirection = EArrowDirection_Left;
				l_bStateUpdated = true;
				

				break;

			case OVTK_GDF_Right:
				m_eCurrentState = EVisualInterfaceState_Cue;
				//test = true;
				m_eCurrentDirection = EArrowDirection_Right;

				l_bStateUpdated = true;

				break;


			case OVTK_GDF_Feedback_Continuous:
				// New trial starts

				m_eCurrentState = EVisualInterfaceState_ContinousFeedback;


				// as some trials may have artifacts and hence very high responses from e.g. LDA
				// its better to reset the max between trials

				l_bStateUpdated = true;

				break;
			}
			
			if (l_bStateUpdated)
			{
				processState();
			}



			// Queue the stimulation to be sent to TCP Tagging
			//m_vStimuliQueue.push_back(m_ui64LastStimulation);
		}

		void CBoxAlgorithmVisualInterface::processState()
		{
			switch (m_eCurrentState)
			{
			case EVisualInterfaceState_Reference:
				if (GTK_WIDGET(m_pDrawingArea)->window)
					gdk_window_invalidate_rect(GTK_WIDGET(m_pDrawingArea)->window,
					NULL,
					true);
				break;

			case EVisualInterfaceState_Cue:
				if (GTK_WIDGET(m_pDrawingArea)->window)
					gdk_window_invalidate_rect(GTK_WIDGET(m_pDrawingArea)->window,
					NULL,
					true);
				break;

			case EVisualInterfaceState_Idle:
				if (GTK_WIDGET(m_pDrawingArea)->window)
					gdk_window_invalidate_rect(GTK_WIDGET(m_pDrawingArea)->window,
					NULL,
					true);
				break;

			case EVisualInterfaceState_ContinousFeedback:
				if (GTK_WIDGET(m_pDrawingArea)->window)
					gdk_window_invalidate_rect(GTK_WIDGET(m_pDrawingArea)->window,
					NULL,
					true);
				break;

			default:
				break;
			}
		}




		void CBoxAlgorithmVisualInterface::MatrixDistanceLDA(const IMatrix* l_pMatrix)
		{
			
			bool essai = false;
			uint32 Dimension = l_pMatrix->getDimensionCount();
			float Ampli = 0.0;
			const float64 * Data = l_pMatrix->getBuffer();
			float64 Ampl1 = 0;
			float64 Ampl2 = 0;
			
			if (m_bTwoValueInput)
			{
				if (Data[0] != 0) {
					Ampl1 = Data[0];
				}

				if (Data[1] != 0) {
					Ampl2 = Data[1];
				}

				Ampli = 50 * (float) (Ampl2 - Ampl1);
				

				//printf("Matrice sortie : %f\n", je);
			}

			else
			{
				Ampli = (float) Data[0];

			}

			

			if (Ampli >= 1)
			{
				Ampli = log(Ampli);
			}
			if (Ampli <= -1)
			{
				Ampli = -log(abs(Ampli));
			}

			if ((Ampli > -1) && (Ampli < 1))
			{
				Ampli = 0;
			}


			if (m_bShowFeedback && !m_bDelayFeedback)
			{	
				if (maxAmpl < Ampli)
				{
					maxAmpl = Ampli;
				}

				while ((maxAmpl*Thresh > 520) || (Thresh > 70))
				{
					Thresh--;
				}

				m_fvelocity = Thresh*Ampli;
				
				//printf("Matrice sortie : %f\n", m_fvelocity);

				switch (m_eCurrentState)
				{
				case EVisualInterfaceState_Reference:
					if (GTK_WIDGET(m_pDrawingArea)->window)
						gdk_window_invalidate_rect(GTK_WIDGET(m_pDrawingArea)->window,
						NULL,
						true);
					break;

				case EVisualInterfaceState_Cue:
					
					if (GTK_WIDGET(m_pDrawingArea)->window)
						gdk_window_invalidate_rect(GTK_WIDGET(m_pDrawingArea)->window,
						NULL,
						true);
					break;

				case EVisualInterfaceState_Idle:
					m_oForegroundColor.red = 0x8000;
					m_oForegroundColor.green = 0x8000;
					m_oForegroundColor.blue = 0x8000;

					if (m_bShowFeedback && m_bDelayFeedback)
					{
						drawBall();
					}
					if (GTK_WIDGET(m_pDrawingArea)->window)
						gdk_window_invalidate_rect(GTK_WIDGET(m_pDrawingArea)->window,
						NULL,
						true);
					
					break;

				case EVisualInterfaceState_ContinousFeedback:
					
					if (m_bShowFeedback && !m_bDelayFeedback)
					{
						drawBall();
					}
					
					if (GTK_WIDGET(m_pDrawingArea)->window)
						gdk_window_invalidate_rect(GTK_WIDGET(m_pDrawingArea)->window,
						NULL,
						true);
					
					break;

				default:
					break;
				}
				
			}
			

		}




		//Constructor

		CBoxAlgorithmVisualInterface::CBoxAlgorithmVisualInterface(void) :
			m_pBuilderInterface(NULL),
			m_pMainWindow(NULL),
			m_eCurrentState(EVisualInterfaceState_Idle),
			m_eCurrentDirection(EArrowDirection_None),
			m_pDrawingArea(NULL),
			m_pLeftHand(NULL),
			m_pLeftHandmodified(NULL),
			m_bShowFeedback(true),
			PosX(0),
			PosY(0),
			Thresh(90),
			maxAmpl(0),
			m_bDelayFeedback(false),
			m_bTwoValueInput(false),
			m_fvelocity(0.0),
			m_bFullScreen(false),
			m_i64PredictionsToIntegrate(5),
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

		bool CBoxAlgorithmVisualInterface::initialize(void)
		{
			m_bShowInstruction = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 0);
			m_bShowFeedback = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 1);
			m_bDelayFeedback = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 2);
			//m_i64PredictionsToIntegrate = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 4);
			m_bFullScreen = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 6);


			m_uiIdleFuncTag = 0;
			m_pStimulusSender = nullptr;
			
			maxAmpl = 0;
			Thresh = 90;
			m_vStimuliQueue.clear();

			m_oInput0Decoder.initialize(*this, 0);
			m_oInput1Decoder.initialize(*this, 1);


			//load the gtk builder interface

			m_pBuilderInterface = gtk_builder_new(); // glade_xml_new(OpenViBE::Directories::getDataDir() + "/plugins/simple-visualization/openvibe-simple-visualization-GrazVisualization.ui", NULL, NULL);
			gtk_builder_add_from_file(m_pBuilderInterface, OpenViBE::Directories::getDataDir() + "/plugins/signal-processing/openvibe-simple-visualization-VisualInterface.ui", NULL);

			if (!m_pBuilderInterface)
			{
				this->getLogManager() << LogLevel_Error << "Error: couldn't load the interface!";
				return false;
			}

			gtk_builder_connect_signals(m_pBuilderInterface, NULL);

			m_pDrawingArea = GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "VisualInterfaceDrawingArea"));
			g_signal_connect(G_OBJECT(m_pDrawingArea), "expose_event", G_CALLBACK(VisualInterface_RedrawCallback), this);
			g_signal_connect(G_OBJECT(m_pDrawingArea), "size-allocate", G_CALLBACK(VisualInterface_SizeAllocateCallback), this);

			//set widget bg color
			gtk_widget_modify_bg(m_pDrawingArea, GTK_STATE_NORMAL, &m_oBackgroundColor);
			gtk_widget_modify_bg(m_pDrawingArea, GTK_STATE_PRELIGHT, &m_oBackgroundColor);
			gtk_widget_modify_bg(m_pDrawingArea, GTK_STATE_ACTIVE, &m_oBackgroundColor);

			gtk_widget_modify_fg(m_pDrawingArea, GTK_STATE_NORMAL, &m_oForegroundColor);
			gtk_widget_modify_fg(m_pDrawingArea, GTK_STATE_PRELIGHT, &m_oForegroundColor);
			gtk_widget_modify_fg(m_pDrawingArea, GTK_STATE_ACTIVE, &m_oForegroundColor);

			m_pLeftHand = gdk_pixbuf_new_from_file_at_size(OpenViBE::Directories::getDataDir() + "/plugins/signal-processing/main.jpg", -1, -1, NULL);

			


			PosX = (m_pDrawingArea->allocation.width) / 12;
			PosY = (m_pDrawingArea->allocation.height) / 2;





			if (!m_pLeftHand)
			{
				this->getLogManager() << LogLevel_Error << "Error couldn't load arrow resource files!\n";

				return false;
			}

			m_pStimulusSender = TCPTagging::createStimulusSender();

			if (!m_pStimulusSender->connect("localhost", "15361"))
			{
				this->getLogManager() << LogLevel_Warning << "Unable to connect to AS's TCP Tagging plugin, stimuli wont be forwarded.\n";
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

		bool CBoxAlgorithmVisualInterface::uninitialize(void)
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


			if (m_pLeftHandmodified){ g_object_unref(G_OBJECT(m_pLeftHandmodified)); }
			if (m_pLeftHand){ g_object_unref(G_OBJECT(m_pLeftHand)); }


			return true;
		}
		/*******************************************************************************/

		bool CBoxAlgorithmVisualInterface::processInput(uint32 ui32InputIndex)
		{
			getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
			return true;
		}


		bool CBoxAlgorithmVisualInterface::process(void)
		{

			// the static box context describes the box inputs, outputs, settings structures
			const IBox& l_rStaticBoxContext = this->getStaticBoxContext();
			// the dynamic box context describes the current state of the box inputs and outputs (i.e. the chunks)
			IBoxIO& l_rDynamicBoxContext = this->getDynamicBoxContext();

			IBoxIO* l_pBoxIO = getBoxAlgorithmContext()->getDynamicBoxContext();



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


			for (uint32 chunk = 0; chunk<l_pBoxIO->getInputChunkCount(1); chunk++)
			{
				m_oInput1Decoder.decode(chunk);
				if (m_oInput1Decoder.isHeaderReceived())
				{
					const IMatrix* l_pMatrix = m_oInput1Decoder.getOutputMatrix();

					if (l_pMatrix->getDimensionCount() == 0)
					{
						this->getLogManager() << LogLevel_Error << "Error, dimension count is 0 for Amplitude input !\n";
						return false;
					}

					if (l_pMatrix->getDimensionCount() > 1)
					{
						for (uint32 k = 1; k<l_pMatrix->getDimensionSize(k); k++)
						{
							if (l_pMatrix->getDimensionSize(k) > 1)
							{
								this->getLogManager() << LogLevel_Error << "Error, only column vectors supported as Amplitude!\n";
								return false;
							}
						}
					}

					if (l_pMatrix->getDimensionSize(0) == 0)
					{
						this->getLogManager() << LogLevel_Error << "Error, need at least 1 dimension in Amplitude input !\n";
						return false;
					}
					else if (l_pMatrix->getDimensionSize(0) >= 2)
					{
						this->getLogManager() << LogLevel_Trace << "Got 2 or more dimensions for feedback, feedback will be the difference between the first two.\n";
						m_bTwoValueInput = true;
					}
				}

				if (m_oInput1Decoder.isBufferReceived())
				{	
					MatrixDistanceLDA(m_oInput1Decoder.getOutputMatrix());
					
				}

			}

			if (m_uiIdleFuncTag == 0)
			{
				m_uiIdleFuncTag = g_idle_add(flush, this);
			}



			// Tutorials:
			// http://openvibe.inria.fr/documentation/#Developer+Documentation
			// Codec Toolkit page :
			// http://openvibe.inria.fr/codec-toolkit-references/

			// Feel free to ask experienced developers on the forum (http://openvibe.inria.fr/forum) and IRC (#openvibe on irc.freenode.net).

			return true;
		}
		void CBoxAlgorithmVisualInterface::resize(uint32 ui32Width, uint32 ui32Height)
		{

			ui32Width = (ui32Width<8 ? 8 : ui32Width);
			ui32Height = (ui32Height<8 ? 8 : ui32Height);

			if (m_pLeftHandmodified)
			{
				g_object_unref(G_OBJECT(m_pLeftHandmodified));
			}

			m_pLeftHandmodified = gdk_pixbuf_scale_simple(m_pLeftHand, (2 * ui32Width) / 8, ui32Height / 4, GDK_INTERP_BILINEAR);


		}

		void CBoxAlgorithmVisualInterface::redraw()
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
			case EVisualInterfaceState_Cue:

				test = false;
				drawTargets(m_bShowInstruction ? m_eCurrentDirection : EArrowDirection_None);
				PosX = l_iWindowWidth / 12;
				break;
			

			case EVisualInterfaceState_Reference:


				gdk_draw_arc(m_pDrawingArea->window,
					m_pDrawingArea->style->fg_gc[GTK_WIDGET_STATE(m_pDrawingArea)],
					true,
					l_iWindowWidth / 2,
					l_iWindowHeight/2,
					l_iWindowWidth / 20,
					l_iWindowWidth / 20,
					1,
					36000);
				
				break;

			}


		}

		void CBoxAlgorithmVisualInterface::drawBall()
		{
			

			gint l_iWindowWidth = m_pDrawingArea->allocation.width;
			gint l_iWindowHeight = m_pDrawingArea->allocation.height;
			
			gdk_gc_set_line_attributes(m_pDrawingArea->style->fg_gc[GTK_WIDGET_STATE(m_pDrawingArea)], 1, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_BEVEL);

			BolePosition(m_fvelocity);
			
			
			gdk_draw_arc(m_pDrawingArea->window,
				m_pDrawingArea->style->fg_gc[GTK_WIDGET_STATE(m_pDrawingArea)],
				true,
				PosX,
				PosY,
				l_iWindowWidth / 20,
				l_iWindowWidth / 20,
				1,
				36000);
			
			
		}

		void CBoxAlgorithmVisualInterface::BolePosition(float Velocity)
		{	
			gint l_iWindowWidth = m_pDrawingArea->allocation.width;
			PosX = PosX + 25 + 0 * (int)Velocity;
			PosY = (m_pDrawingArea->allocation.height)/2;
			PosY = PosY + (int)Velocity;
			
		}

		


		

		void CBoxAlgorithmVisualInterface::drawTargets(EArrowDirection eDirection)
		{	

			m_oForegroundColor.pixel = 1;
			m_oForegroundColor.red = 0x8000;
			m_oForegroundColor.green = 0x8000;
			m_oForegroundColor.blue = 0x8000;
			gint l_iWindowWidth = m_pDrawingArea->allocation.width;
			gint l_iWindowHeight = m_pDrawingArea->allocation.height;

			gint cpt = 0;


			//increase line's width
			gdk_gc_set_line_attributes(m_pDrawingArea->style->fg_gc[GTK_WIDGET_STATE(m_pDrawingArea)], 1, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_BEVEL);

			switch (eDirection)
			{
			case EArrowDirection_None:
				//horizontal line
				gdk_draw_line(m_pDrawingArea->window,
					m_pDrawingArea->style->fg_gc[GTK_WIDGET_STATE(m_pDrawingArea)],
					(l_iWindowWidth / 4), (l_iWindowHeight / 2),
					((3 * l_iWindowWidth) / 4), (l_iWindowHeight / 2)
					);
				break;

			case EArrowDirection_Left:
				
				gdk_draw_rectangle(m_pDrawingArea->window,
					m_pDrawingArea->style->fg_gc[GTK_WIDGET_STATE(m_pDrawingArea)],
					true,
					(l_iWindowWidth * 1900 / 1920),
					(l_iWindowHeight * 10 / 100),
					(l_iWindowWidth * 16 / 16),
					(l_iWindowHeight * 30 / 100));

				break;

			case EArrowDirection_Right:
				gdk_draw_rectangle(m_pDrawingArea->window,
					m_pDrawingArea->style->fg_gc[GTK_WIDGET_STATE(m_pDrawingArea)],
					true,
					(l_iWindowWidth * 1900 / 1920),
					(l_iWindowHeight * 70 / 100),
					(l_iWindowWidth * 16 / 16),
					(l_iWindowHeight * 90 / 100));

				break;
			}
		}



		void CBoxAlgorithmVisualInterface::drawReferenceCross()
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

		void CBoxAlgorithmVisualInterface::flushQueue(void)
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
