/*
 * Copyright (C) 2014
 * Martin Lambers <marlam@marlam.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <cstdlib>

#include <stdexcept>

#include <GL/glew.h>
#include <eq/eq.h>

#include "base/dbg.h"
#include "base/msg.h"
#include "base/ser.h"

#include "xgl/glvm.hpp"
#include "xgl/glvm-ser.hpp"
#include "xgl/glcontext.hpp"
#include "xgl/glrenderer.hpp"

#include "tracking.hpp"
#include "tracking-eqevent.hpp"

#include "eqwindow.hpp"


static GLRendererFactory* global_glrenderer_factory = NULL;


class eq_init_data : public co::Object
{
public:
    eq::uint128_t frame_data_id;
    msg::level_t msg_level;

    eq_init_data()
    {
    }

protected:
    virtual ChangeType getChangeType() const
    {
        return co::Object::STATIC;
    }

    virtual void getInstanceData(co::DataOStream& os)
    {
        std::ostringstream oss;
        s11n::save(oss, frame_data_id.high());
        s11n::save(oss, frame_data_id.low());
        s11n::save(oss, static_cast<int>(msg_level));
        os << oss.str();
    }

    virtual void applyInstanceData(co::DataIStream& is)
    {
        int x;
        std::string s;
        is >> s;
        std::istringstream iss(s);
        s11n::load(iss, frame_data_id.high());
        s11n::load(iss, frame_data_id.low());
        s11n::load(iss, x); msg_level = static_cast<msg::level_t>(x);
    }
};

class eq_frame_data : public co::Object
{
public:
    GLContext glcontext;
    bool scene_is_2d;
    glvm::frust frustum;
    glvm::vec2 translation_2d;
    glvm::vec3 scale_2d;
    glvm::vec3 viewer_pos;
    glvm::quat viewer_rot;
    glvm::vec3 tracker_pos;
    glvm::quat tracker_rot;
    bool statistics_overlay;

    eq_frame_data() :
        glcontext(global_glrenderer_factory),
        scene_is_2d(false), frustum(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f),
        translation_2d(0.0f), scale_2d(0.0f),
        viewer_pos(0.0f), viewer_rot(0.0f, 0.0f, 0.0f, 0.0f),
        tracker_pos(0.0f), tracker_rot(0.0f, 0.0f, 0.0f, 0.0f),
        statistics_overlay(false)
    {
    }

protected:
    virtual ChangeType getChangeType() const
    {
        return co::Object::INSTANCE;
    }

    virtual void getInstanceData(co::DataOStream &os)
    {
        std::ostringstream oss;
        s11n::save(oss, *glcontext.get_renderer());
        s11n::save(oss, scene_is_2d);
        s11n::save(oss, frustum);
        s11n::save(oss, translation_2d);
        s11n::save(oss, scale_2d);
        s11n::save(oss, viewer_pos);
        s11n::save(oss, viewer_rot);
        s11n::save(oss, tracker_pos);
        s11n::save(oss, tracker_rot);
        os << oss.str();
    }

    virtual void applyInstanceData(co::DataIStream &is)
    {
        std::string s;
        is >> s;
        std::istringstream iss(s);
        s11n::load(iss, *glcontext.get_renderer());
        s11n::load(iss, scene_is_2d);
        s11n::load(iss, frustum);
        s11n::load(iss, translation_2d);
        s11n::load(iss, scale_2d);
        s11n::load(iss, viewer_pos);
        s11n::load(iss, viewer_rot);
        s11n::load(iss, tracker_pos);
        s11n::load(iss, tracker_rot);
    }
};

class eq_config : public eq::Config
{
private:
    eq_init_data _eq_init_data;         // Master eq_init_data instance
    eq_frame_data _eq_frame_data;       // Master eq_frame_data instance
    const GLNavigator* _navigator;
#if 0
    TrackingDriverDTrack* _tracking_driver_dtrack;
#endif
    TrackingDriverEQEvent* _tracking_driver_eqevent;
    Tracking* _tracking;
    int _flystick_handle;
    int _viewer_handle;
    bool _quit_request;

public:
    eq_config(eq::ServerPtr parent) :
        eq::Config(parent),
        _eq_init_data(), _eq_frame_data(),
        _navigator(NULL),
#if 0
        _tracking_driver_dtrack(NULL),
#endif
        _tracking_driver_eqevent(NULL),
        _tracking(NULL),
        _quit_request(false)
    {
    }

    ~eq_config()
    {
        delete _tracking;
#if 0
        delete _tracking_driver_dtrack;
#endif
        delete _tracking_driver_eqevent;
    }

    bool init(msg::level_t msg_level, const GLNavigator* navigator, int tracking)
    {
        _navigator = navigator;
        _eq_init_data.msg_level = msg_level;
        // Register master instances
        registerObject(&_eq_frame_data);
        _eq_init_data.frame_data_id = _eq_frame_data.getID();
        registerObject(&_eq_init_data);
        // Initialize tracking
        if (tracking) {
#if 0
            if (tracking == 1) {
                _tracking_driver_eqevent = new TrackingDriverEQEvent();
                _tracking = new Tracking(_tracking_driver_eqevent);
            } else {
                _tracking_driver_dtrack = new TrackingDriverDTrack();
                _tracking = new Tracking(_tracking_driver_dtrack);
            }
#else
            _tracking_driver_eqevent = new TrackingDriverEQEvent();
            _tracking = new Tracking(_tracking_driver_eqevent);
#endif
            _flystick_handle = _tracking->track(Tracking::FLYSTICK, 0);
            _tracking->set_auto_repeat(_flystick_handle, 1);
            _tracking->set_auto_repeat(_flystick_handle, 2);
            _tracking->set_auto_repeat(_flystick_handle, 3);
            _tracking->set_auto_repeat(_flystick_handle, 4);
            _viewer_handle = _tracking->track(Tracking::BODY, 0);
        }
        return eq::Config::init(_eq_init_data.getID());
    }

    GLContext* get_glcontext()
    {
        return &_eq_frame_data.glcontext;
    }

    virtual bool exit()
    {
        bool ret = eq::Config::exit();
        // Deregister master instances
        deregisterObject(&_eq_init_data);
        deregisterObject(&_eq_frame_data);
        return ret;
    }

    virtual uint32_t startFrame()
    {
        // Navigation
        if (_navigator) {
            _eq_frame_data.scene_is_2d = _navigator->scene_is_2d();
            if (_eq_frame_data.scene_is_2d) {
                float ar = 1.0f;
                if (getCanvases().size() > 0) {
                    ar = static_cast<float>(getCanvases()[0]->getWall().getWidth())
                        / getCanvases()[0]->getWall().getHeight();
                }
                float l = -1.0f, r = +1.0f, b = -1.0f, t = +1.0f;
                if (ar > 1.0f) {
                    l = -ar;
                    r = +ar;
                } else if (ar < 1.0f) {
                    b = -(1.0f / ar);
                    t = +(1.0f / ar);
                }
                _eq_frame_data.frustum = glvm::frust(l, r, b, t, -1.0f, +1.0f);
                _navigator->scene_view_2d(
                        _eq_frame_data.translation_2d, _eq_frame_data.scale_2d);
            } else {
                float focal_length;     // ignored: stereo rendering is configured via Equalizer
                float eye_separation;   // ignored: stereo rendering is configured via Equalizer
                _navigator->scene_view_3d(_eq_frame_data.frustum,
                        _eq_frame_data.viewer_pos, _eq_frame_data.viewer_rot,
                        focal_length, eye_separation);
            }
        }

        // Tracking
        if (_tracking) {
            _tracking->update();
            if (_tracking->up_to_date(_viewer_handle)) {
                _eq_frame_data.tracker_pos = _tracking->pos(_viewer_handle);
                _eq_frame_data.tracker_rot = glvm::toQuat(_tracking->rot(_viewer_handle));
            }
        }

        // Set viewer head position and orientation
        glvm::mat4 head_tracking_matrix = glvm::toMat4(_eq_frame_data.tracker_rot);
        translation(head_tracking_matrix) = _eq_frame_data.tracker_pos;
        eq::Matrix4f eq_head_matrix;
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                eq_head_matrix.array[i * 4 + j] = head_tracking_matrix[i][j];
        getObservers().at(0)->setHeadMatrix(eq_head_matrix);

        // Commit new version of updated frame data
        const eq::uint128_t version = _eq_frame_data.commit();

        // Start this frame with the committed frame data
        return eq::Config::startFrame(version);
    }

    virtual bool handleEvent(const eq::ConfigEvent* event)
    {
        if (event->data.type == eq::Event::KEY_PRESS
                && event->data.keyPress.key == eq::KC_F12) {
            _eq_frame_data.statistics_overlay = !_eq_frame_data.statistics_overlay;
            return true;
        }
        if (eq::Config::handleEvent(event)) {
            return true;
        }
        if (_tracking_driver_eqevent && _tracking_driver_eqevent->handle_event(event)) {
            return true;
        }
        return false;
    }
};

class eq_node : public eq::Node
{
public:
    class eq_init_data eq_init_data;

    eq_node(eq::Config* parent) : eq::Node(parent)
    {
    }

protected:
    virtual bool configInit(const eq::uint128_t& init_id)
    {
        if (!eq::Node::configInit(init_id)) {
            return false;
        }
        // Map our InitData instance to the master instance
        eq_config* config = static_cast<eq_config*>(getConfig());
        if (!config->mapObject(&eq_init_data, init_id)) {
            return false;
        }
        msg::set_level(eq_init_data.msg_level);
        return true;
    }

    virtual bool configExit()
    {
        eq::Config* config = getConfig();
        // Unmap our InitData instance
        config->unmapObject(&eq_init_data);
        // Cleanup
        return eq::Node::configExit();
    }
};

class eq_pipe : public eq::Pipe
{
public:
    class eq_frame_data eq_frame_data;
    bool shared_init_done;
    bool shared_prerender_done;
    int shared_postrender_countdown;
    int shared_exit_countdown;

    eq_pipe(eq::Node* parent) :
        eq::Pipe(parent),
        eq_frame_data()
    {
    }

protected:
    virtual bool configInit(const eq::uint128_t& init_id)
    {
        shared_init_done = false;
        if (!eq::Pipe::configInit(init_id)) {
            return false;
        }
        // Map our FrameData instance to the master instance
        eq_config* config = static_cast<eq_config*>(getConfig());
        eq_node* node = static_cast<eq_node*>(getNode());
        if (!config->mapObject(&eq_frame_data, node->eq_init_data.frame_data_id)) {
            return false;
        }
        return true;
    }

    virtual bool configExit()
    {
        eq::Config* config = getConfig();
        // Unmap our FrameData instance
        config->unmapObject(&eq_frame_data);
        // Cleanup
        return eq::Pipe::configExit();
    }

    virtual void frameStart(const eq::uint128_t& frame_id, const uint32_t frame_number)
    {
        // Update our frame data
        eq_frame_data.sync(frame_id);
        // Prepare per-shared-context actions
        shared_prerender_done = false;
        shared_postrender_countdown = 0;
        for (size_t i = 0; i < getWindows().size(); i++)
            shared_postrender_countdown += getWindows()[i]->getChannels().size();
        shared_exit_countdown = getWindows().size();
        eq::Pipe::frameStart(frame_id, frame_number);
    }

    virtual void frameFinish(const eq::uint128_t& frame_id, const uint32_t frame_number)
    {
        assert(shared_postrender_countdown == 0);
        eq::Pipe::frameFinish(frame_id, frame_number);
    }
};

class eq_window : public eq::Window
{
public:
#if 0
    DynCalib dyncalib;
#endif
    bool window_prerender_done;
    int window_postrender_countdown;

    eq_window(eq::Pipe* parent) :
        eq::Window(parent)
    {
    }

protected:
    virtual bool configInitGL(const eq::uint128_t& init_id)
    {
        if (!eq::Window::configInitGL(init_id))
            return false;

        // Disable some things that Equalizer seems to enable for some reason.
        glDisable(GL_LIGHTING);

        // Initialize
#if 0
        dyncalib.init_gl(getName(), getPixelViewport().w, getPixelViewport().h);
#endif

        class eq_pipe* eq_pipe = static_cast<class eq_pipe*>(getPipe());
        GLRenderer* glrenderer = eq_pipe->eq_frame_data.glcontext.get_renderer();
        try {
            if (!eq_pipe->shared_init_done) {
                glrenderer->init_gl_shared();
                eq_pipe->shared_init_done = true;
            }
            glrenderer->init_gl_window();
        }
        catch (std::exception& e) {
            msg::err("Failure in configInitGL(): %s", e.what());
            return false;
        }

        return true;
    }

    virtual bool configExitGL()
    {
        class eq_pipe* eq_pipe = static_cast<class eq_pipe*>(getPipe());
        GLRenderer* glrenderer = eq_pipe->eq_frame_data.glcontext.get_renderer();
        try {
            glrenderer->exit_gl_window();
            assert(eq_pipe->shared_exit_countdown > 0);
            eq_pipe->shared_exit_countdown--;
            if (eq_pipe->shared_exit_countdown == 0) {
                glrenderer->exit_gl_shared();
            }
#if 0
            dyncalib.exit_gl();
#endif
        }
        catch (std::exception& e) {
            msg::err("Failure in configExitGL(): %s", e.what());
            return false;
        }
        return eq::Window::configExitGL();
    }

    virtual void frameStart(const eq::uint128_t& frame_id, const uint32_t frame_number)
    {
        window_prerender_done = false;
        window_postrender_countdown = getChannels().size();
        eq::Window::frameStart(frame_id, frame_number);
    }

    virtual void frameDrawFinish(const eq::uint128_t& frame_id, const uint32_t frame_number)
    {
#if 0
        class eq_pipe* eq_pipe = static_cast<class eq_pipe*>(getPipe());
        dyncalib.apply(eq_pipe->eq_frame_data.tracker_pos);
#endif
        eq::Window::frameDrawFinish(frame_id, frame_number);
    }

    virtual void frameFinish(const eq::uint128_t& frame_id, const uint32_t frame_number)
    {
        assert(window_postrender_countdown == 0);
        eq::Window::frameFinish(frame_id, frame_number);
    }
};

class eq_channel : public eq::Channel
{
public:
    eq_channel(eq::Window *parent) :
        eq::Channel(parent)
    {
    }

protected:

    virtual void frameStart(const eq::uint128_t& frame_id, const uint32_t frame_number)
    {
        class eq_pipe* eq_pipe = static_cast<class eq_pipe*>(getPipe());
        class eq_window* eq_window = static_cast<class eq_window*>(getWindow());
        GLRenderer* glrenderer = eq_pipe->eq_frame_data.glcontext.get_renderer();
        try {
            if (!eq_pipe->shared_prerender_done) {
                glrenderer->pre_render_shared();
                eq_pipe->shared_prerender_done = true;
            }
            if (!eq_window->window_prerender_done) {
                glrenderer->pre_render_window();
                eq_window->window_prerender_done = true;
            }
        }
        catch (std::exception& e) {
            msg::err("Failure in frameStart(): %s", e.what());
            abort();
        }
        eq::Channel::frameStart(frame_id, frame_number);
    }

    virtual void frameDraw(const eq::uint128_t& frame_id)
    {
        class eq_pipe* eq_pipe = static_cast<class eq_pipe*>(getPipe());
        class eq_frame_data& eq_frame_data = eq_pipe->eq_frame_data;
        try {
            eq::Channel::frameDraw(frame_id);
            if (eq_frame_data.scene_is_2d) {
                const eq::Viewport& relf = getViewport();
                const glvm::frust& oldf = eq_frame_data.frustum;
                glvm::frust newf;
                newf.l = oldf.l + relf.x * (oldf.r - oldf.l);
                newf.r = newf.l + relf.w * (oldf.r - oldf.l);
                newf.b = oldf.b + relf.y * (oldf.t - oldf.b);
                newf.t = newf.b + relf.h * (oldf.t - oldf.b);
                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();
                glOrtho(newf.l, newf.r, newf.b, newf.t, oldf.n, oldf.f);
                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();
                glTranslatef(eq_frame_data.translation_2d.x, eq_frame_data.translation_2d.y, 0.0f);
                glScalef(eq_frame_data.scale_2d.x, eq_frame_data.scale_2d.y, eq_frame_data.scale_2d.z);
            } else {
                /* Ignore the frustum, we use the one provided by Equalizer
                glMatrixMode(GL_PROJECTION);
                glLoadMatrixf(glvm::toMat4(eq_frame_data.frustum));
                */
                glMatrixMode(GL_MODELVIEW);
                glLoadMatrixf(glvm::translate(glvm::toMat4(-eq_frame_data.viewer_rot), -eq_frame_data.viewer_pos));
            }
            GLRenderer* glrenderer = eq_frame_data.glcontext.get_renderer();
            glrenderer->render();
        }
        catch (std::exception& e) {
            msg::err("Failure in frameDraw(): %s", e.what());
            abort();
        }
    }

    virtual void frameDrawFinish(const eq::uint128_t& frame_id, const uint32_t frame_number)
    {
        class eq_pipe* eq_pipe = static_cast<class eq_pipe*>(getPipe());
        class eq_window* eq_window = static_cast<class eq_window*>(getWindow());
        GLRenderer* glrenderer = eq_pipe->eq_frame_data.glcontext.get_renderer();
        try {
            assert(eq_window->window_postrender_countdown > 0);
            eq_window->window_postrender_countdown--;
            if (eq_window->window_postrender_countdown == 0) {
                glrenderer->post_render_window();
            }
            assert(eq_pipe->shared_postrender_countdown > 0);
            eq_pipe->shared_postrender_countdown--;
            if (eq_pipe->shared_postrender_countdown == 0) {
                glrenderer->post_render_shared();
            }
        }
        catch (std::exception& e) {
            msg::err("Failure in frameDrawFinish(): %s", e.what());
            abort();
        }
        eq::Channel::frameDrawFinish(frame_id, frame_number);
    }

    virtual void frameViewFinish(const eq::uint128_t& frame_id)
    {
        class eq_pipe* eq_pipe = static_cast<class eq_pipe*>(getPipe());
        if (eq_pipe->eq_frame_data.statistics_overlay)
            drawStatistics();
        eq::Channel::frameViewFinish(frame_id);
    }
};

class eq_node_factory : public eq::NodeFactory
{
public:
    virtual eq::Config* createConfig(eq::ServerPtr parent) { return new eq_config(parent); }
    virtual eq::Node* createNode(eq::Config* parent) { return new eq_node(parent); }
    virtual eq::Pipe* createPipe(eq::Node* parent) { return new eq_pipe(parent); }
    virtual eq::Window* createWindow(eq::Pipe* parent) { return new eq_window(parent); }
    virtual eq::Channel* createChannel(eq::Window* parent) { return new eq_channel(parent); }
};

EQWindow::EQWindow(GLRendererFactory* renderer_factory, const GLNavigator* navigator, int tracking, int *argc, char *argv[]) :
    GLWindow(NULL), _valid(false)
{
    /* Initialize our renderer factory singleton */
    global_glrenderer_factory = renderer_factory;
    /* Initialize Equalizer */
    _eq_node_factory = new eq_node_factory;
    if (!eq::init(*argc, argv, _eq_node_factory)) {
        throw std::runtime_error("Equalizer initialization failed.");
    }
    /* Get a configuration */
    _eq_config = static_cast<eq_config*>(eq::getConfig(*argc, argv));
    // The following code is only executed on the application node because
    // eq::getConfig() does not return on other nodes.
    if (!_eq_config) {
        throw std::runtime_error("Cannot get Equalizer configuration.");
    }
    if (!_eq_config->init(msg::level(), navigator, tracking)) {
        throw std::runtime_error("Cannot initialize Equalizer configuration.");
    }
    /* Initialize GLWindow */
    set_shared_context(_eq_config->get_glcontext());
    _valid = true;
}

EQWindow::~EQWindow()
{
    if (_valid && _eq_config) {
        _eq_config->exit();
        eq::releaseConfig(_eq_config);
        eq::exit();
    }
    delete _eq_node_factory;
}

void EQWindow::render()
{
    if (running()) {
        _eq_config->startFrame();
        _eq_config->finishFrame();
    }
}

bool EQWindow::running() const
{
    return (_valid && _eq_config && _eq_config->isRunning());
}
