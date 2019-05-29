#ifndef __OpenViBEPlugins_BoxAlgorithm_Hand_H__
#define __OpenViBEPlugins_BoxAlgorithm_Hand_H__

//You may have to change this path to match your folder organisation
#include "../ovp_defines.h"

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>


#include <visualization-toolkit/ovviz_all.h>

#include <gtk/gtk.h>

#include <vector>
#include <string>
#include <map>
#include <deque>


// The unique identifiers for the box and its descriptor.
// Identifier are randomly chosen by the skeleton-generator.
#define OVP_ClassId_BoxAlgorithm_Hand (0x9043ece1, 0xc2dc5b71)
#define OVP_ClassId_BoxAlgorithm_HandDesc (0xad428a6b, 0xa842ddd6)
#define OV_AttributeId_Box_FlagIsUnstable OpenViBE::CIdentifier(0x666FFFFF, 0x666FFFFF)

namespace TCPTagging
{
	class IStimulusSender; // fwd declare
};
namespace OpenViBEPlugins
{
	namespace SignalProcessing
	{
		/**
		* \class CBoxAlgorithmVisualInterface
		* \author Tristan (Sorbonne Université)
		* \date Tue Feb 12 14:49:06 2019
		* \brief The class CBoxAlgorithmVisualInterface describes the box VisualInterface.
		*
		*/
		enum EArrowDirectionHand
		{
			EArrowDirectionHand_None = 0,
			EArrowDirectionHand_Left,
			EArrowDirectionHand_Right,
			EArrowDirectionHand_Up,
			EArrowDirectionHand_Down,
		};

		enum EHandInterfaceState
		{
			EHandInterfaceState_Idle,
			EHandInterfaceState_Reference,
			EHandInterfaceState_Cue,
			EHandInterfaceState_ContinousFeedback
		};



		class CBoxAlgorithmHand : virtual public OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >
		{
		public:

			CBoxAlgorithmHand(void);

			virtual void release(void) { delete this; }

			virtual bool initialize(void);
			virtual bool uninitialize(void);
			virtual bool processInput(OpenViBE::uint32 ui32InputIndex);

			//Here is the different process callbacks possible
			// - On clock ticks :
			//virtual bool processClock(OpenViBE::CMessageClock& rMessageClock);		
			// - On new input received (the most common behaviour for signal processing) :
			//virtual bool processInput(uint32_t ui32InputIndex);

			// If you want to use processClock, you must provide the clock frequency.
			//virtual uint64_t getClockFrequency(void);

			virtual bool process(void);

			virtual void redraw();
			

			virtual void resize(OpenViBE::uint32 ui32Width, OpenViBE::uint32 ui32Height);



			void flushQueue(void);
			// Variables used for the motion of the ball.

			

			
			
			

		 	

			// As we do with any class in openvibe, we use the macro below 
			// to associate this box to an unique identifier. 
			// The inheritance information is also made available, 
			// as we provide the superclass OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >
			// Input decoder:
			

		protected:
			
			// No Output decoder.


			virtual void setStimulation(const OpenViBE::uint32 ui32StimulationIndex, const OpenViBE::uint64 ui64StimulationIdentifier, const OpenViBE::uint64 ui64StimulationDate);
			virtual void drawTargets(EArrowDirectionHand eDirection);
			
			virtual void processState();
			virtual void drawReferenceCross();
			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >, OVP_ClassId_BoxAlgorithm_Hand);

		public:
			//The Builder handler used to create the interface
			::GtkBuilder* m_pBuilderInterface;
			GtkWidget*  m_pMainWindow;
			GtkWidget*  m_pDrawingArea;

			// Input decoder:
			OpenViBEToolkit::TStimulationDecoder < CBoxAlgorithmHand > m_oInput0Decoder;
			OpenViBEToolkit::TStreamedMatrixDecoder < CBoxAlgorithmHand > m_oInput1Decoder;

			EHandInterfaceState m_eCurrentState;
			EArrowDirectionHand m_eCurrentDirection;

		

			GdkPixbuf *m_pActivityHandleft;
			GdkPixbuf *m_pActivityHandleftmodified;


			GdkPixbuf *m_pNeutralHandleft;
			GdkPixbuf *m_pNeutralHandleftmodified;


			GdkPixbuf *m_pActivityHandright;
			GdkPixbuf *m_pActivityHandrightmodified;

			OpenViBE::uint64 m_i64left;

			GdkPixbuf *m_pNeutralHandright;
			GdkPixbuf *m_pNeutralHandrightmodified;

			GdkColor m_oBackgroundColor;
			GdkColor m_oForegroundColor;
			
			int right;
			bool  m_bFullScreen;
			bool m_bShowInstruction;
			bool test;
			bool test2;

			//We save the received stimulations
			OpenViBE::CStimulationSet m_oPendingStimulationSet;

			std::vector< OpenViBE::uint64 > m_vStimuliQueue;
			guint m_uiIdleFuncTag;
			TCPTagging::IStimulusSender* m_pStimulusSender;
		private:
			OpenViBEVisualizationToolkit::IVisualizationContext* m_visualizationContext = nullptr;


		};


		// If you need to implement a box Listener, here is a skeleton for you.
		// Use only the callbacks you need.
		// For example, if your box has a variable number of input, but all of them must be stimulation inputs.
		// The following listener callback will ensure that any newly added input is stimulations :
		/*
		virtual bool onInputAdded(OpenViBE::Kernel::IBox& rBox, const uint32_t ui32Index)
		{
		rBox.setInputType(ui32Index, OV_TypeId_Stimulations);
		};
		*/

		/*
		// The box listener can be used to call specific callbacks whenever the box structure changes : input added, name changed, etc.
		// Please uncomment below the callbacks you want to use.
		class CBoxAlgorithmVisualInterfaceListener : public OpenViBEToolkit::TBoxListener < OpenViBE::Plugins::IBoxListener >
		{
		public:

		//virtual bool onInitialized(OpenViBE::Kernel::IBox& rBox) { return true; };
		//virtual bool onNameChanged(OpenViBE::Kernel::IBox& rBox) { return true; };
		//virtual bool onInputConnected(OpenViBE::Kernel::IBox& rBox, const uint32_t ui32Index) { return true; };
		//virtual bool onInputDisconnected(OpenViBE::Kernel::IBox& rBox, const uint32_t ui32Index) { return true; };
		//virtual bool onInputAdded(OpenViBE::Kernel::IBox& rBox, const uint32_t ui32Index) { return true; };
		//virtual bool onInputRemoved(OpenViBE::Kernel::IBox& rBox, const uint32_t ui32Index) { return true; };
		//virtual bool onInputTypeChanged(OpenViBE::Kernel::IBox& rBox, const uint32_t ui32Index) { return true; };
		//virtual bool onInputNameChanged(OpenViBE::Kernel::IBox& rBox, const uint32_t ui32Index) { return true; };
		//virtual bool onOutputConnected(OpenViBE::Kernel::IBox& rBox, const uint32_t ui32Index) { return true; };
		//virtual bool onOutputDisconnected(OpenViBE::Kernel::IBox& rBox, const uint32_t ui32Index) { return true; };
		//virtual bool onOutputAdded(OpenViBE::Kernel::IBox& rBox, const uint32_t ui32Index) { return true; };
		//virtual bool onOutputRemoved(OpenViBE::Kernel::IBox& rBox, const uint32_t ui32Index) { return true; };
		//virtual bool onOutputTypeChanged(OpenViBE::Kernel::IBox& rBox, const uint32_t ui32Index) { return true; };
		//virtual bool onOutputNameChanged(OpenViBE::Kernel::IBox& rBox, const uint32_t ui32Index) { return true; };
		//virtual bool onSettingAdded(OpenViBE::Kernel::IBox& rBox, const uint32_t ui32Index) { return true; };
		//virtual bool onSettingRemoved(OpenViBE::Kernel::IBox& rBox, const uint32_t ui32Index) { return true; };
		//virtual bool onSettingTypeChanged(OpenViBE::Kernel::IBox& rBox, const uint32_t ui32Index) { return true; };
		//virtual bool onSettingNameChanged(OpenViBE::Kernel::IBox& rBox, const uint32_t ui32Index) { return true; };
		//virtual bool onSettingDefaultValueChanged(OpenViBE::Kernel::IBox& rBox, const uint32_t ui32Index) { return true; };
		//virtual bool onSettingValueChanged(OpenViBE::Kernel::IBox& rBox, const uint32_t ui32Index) { return true; };

		_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxListener < OpenViBE::Plugins::IBoxListener >, OV_UndefinedIdentifier);
		};
		*/

		/**
		* \class CBoxAlgorithmVisualInterfaceDesc
		* \author Tristan (Sorbonne Université)
		* \date Tue Feb 12 14:49:06 2019
		* \brief Descriptor of the box VisualInterface.
		*
		*/
		class CBoxAlgorithmHandDesc : virtual public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			virtual void release(void) { }

			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("Hand"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Tristan"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("Sorbonne Université"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("Hand controlled by thoughts"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString("Control of the Hand due to the detection of a cortex activity"); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Signal processing"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("1.0"); }
			virtual OpenViBE::CString getStockItemName(void) const       { return OpenViBE::CString("gtk-fullscreen"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_BoxAlgorithm_Hand; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::SignalProcessing::CBoxAlgorithmHand; }

			/*
			virtual OpenViBE::Plugins::IBoxListener* createBoxListener(void) const               { return new CBoxAlgorithmVisualInterfaceListener; }
			virtual void releaseBoxListener(OpenViBE::Plugins::IBoxListener* pBoxListener) const { delete pBoxListener; }
			*/
			virtual bool getBoxPrototype(
				OpenViBE::Kernel::IBoxProto& rBoxAlgorithmPrototype) const
			{
				rBoxAlgorithmPrototype.addInput("Stimulations", OV_TypeId_Stimulations);
				rBoxAlgorithmPrototype.addInput("Ampltude", OV_TypeId_StreamedMatrix);

				rBoxAlgorithmPrototype.addFlag(OpenViBE::Kernel::BoxFlag_CanModifyInput);
				rBoxAlgorithmPrototype.addFlag(OpenViBE::Kernel::BoxFlag_CanAddInput);

				//No output specified.To add outputs use :
				//rBoxAlgorithmPrototype.addOutput("OutputName",OV_TypeId_XXXX);

				//rBoxAlgorithmPrototype.addFlag(OpenViBE::Kernel::BoxFlag_CanModifyOutput);
				//rBoxAlgorithmPrototype.addFlag(OpenViBE::Kernel::BoxFlag_CanAddOutput);

				rBoxAlgorithmPrototype.addSetting("Show instruction", OV_TypeId_Boolean, "true");
				rBoxAlgorithmPrototype.addSetting("Show feedback", OV_TypeId_Boolean, "false");
				rBoxAlgorithmPrototype.addSetting("Delay feedback", OV_TypeId_Boolean, "false");
				rBoxAlgorithmPrototype.addSetting("Show accuracy", OV_TypeId_Boolean, "false");
				rBoxAlgorithmPrototype.addSetting("Predictions to integrate", OV_TypeId_Integer, "5");
				rBoxAlgorithmPrototype.addSetting("Positive feedback only", OV_TypeId_Boolean, "false");
				rBoxAlgorithmPrototype.addSetting("Left (1) or Right (2)", OV_TypeId_Integer, "1");
				rBoxAlgorithmPrototype.addSetting("FullScreen", OV_TypeId_Boolean,"true");


				rBoxAlgorithmPrototype.addFlag(OpenViBE::Kernel::BoxFlag_CanModifySetting);
				rBoxAlgorithmPrototype.addFlag(OpenViBE::Kernel::BoxFlag_CanAddSetting);

				rBoxAlgorithmPrototype.addFlag(OV_AttributeId_Box_FlagIsUnstable);

				return true;
			}
			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_BoxAlgorithm_HandDesc);
		};
	};
};

#endif // __OpenViBEPlugins_BoxAlgorithm_VisualInterface_H__
