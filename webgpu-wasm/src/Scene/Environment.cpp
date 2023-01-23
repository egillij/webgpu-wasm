#include "Environment.h"

#include "Application.h"
#include "Renderer/TextureSystem.h"

#include "Renderer/WebGPU/wgpuDevice.h"
#include "Renderer/WebGPU/wgpuTexture.h"
#include "Renderer/WebGPU/wgpuCubemap.h"

#include "Renderer/PreProcessors/CubemapGenerationPipeline.h"


Environment::Environment(const std::string& name, const std::string& filename, WGpuDevice* device)
: m_Name(name), m_Background(nullptr), m_DiffuseIrradiance(nullptr), m_SpecularRadiance(nullptr)
{
    class EnvironmentTextureLoadTask : public TextureLoadTask {
    public:
        EnvironmentTextureLoadTask(Environment* environment, WGpuDevice* device) 
        : m_Environment(environment), m_BackgroundPipeline(nullptr),
          m_DiffusePipeline(nullptr), m_SpecularPipeline(nullptr)
        {
            m_BackgroundPipeline = new CubemapGenerationPipeline(
                                        CubemapGenerationPipeline::PipelineType::EquirectangularToCubemap,
                                        device
                                    );

            // m_DiffusePipeline = new CubemapGenerationPipeline(
            //                             CubemapGenerationPipeline::PipelineType::DiffuseIrradiance,
            //                             device
            //                         );    

        }

        ~EnvironmentTextureLoadTask() 
        {

        }

        virtual void execute(WGpuTexture* texture) override 
        {
            printf("Exectue load task\n");
            //TODO: make cubemap background, diffuse irradiance and specular mips
            m_BackgroundPipeline->process(texture, m_Environment->getBackground());
            // m_DiffusePipeline->process(texture, m_Environment->getDiffuseIrradiance());
        }

    private:
        Environment* m_Environment;
        CubemapGenerationPipeline* m_BackgroundPipeline;
        CubemapGenerationPipeline* m_DiffusePipeline;
        CubemapGenerationPipeline* m_SpecularPipeline;
    };
    
    TextureCreateInfo info{};
    info.format = TextureFormat::RGBA32Float;
    info.width = 512u;
    info.height = 512u;
    info.usage = {TextureUsage::RenderAttachment, TextureUsage::TextureBinding};
    m_Background = new WGpuCubemap(m_Name + "_BackgroundCube", &info, device);
    // m_DiffuseIrradiance = new WGpuCubemap(m_Name + "_DffisueIrradiance", &info, device);

    std::string serverResource = "/resources/environments/" + filename;
    EnvironmentTextureLoadTask* task = new EnvironmentTextureLoadTask(this, device);
    Application::get()->getTextureSystem()->registerTexture(0, m_Name + "_EnvBackground", serverResource, task);
}

Environment::~Environment()
{

}
