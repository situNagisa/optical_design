// MIT License

// Copyright (c) 2019 Erin Catto

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "box2d/draw.h"
#include "imgui/imgui.h"

#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

namespace gl = ngl::gl;

DebugDraw g_debugDraw;
Camera g_camera;

//
Camera::Camera()
{
	m_width = 1280;
	m_height = 800;
	ResetView();
}

//
void Camera::ResetView()
{
	m_center.Set(0.0f, 20.0f);
	m_zoom = 1.0f;
}

//
b2Vec2 Camera::ConvertScreenToWorld(const b2Vec2& ps)
{
	float w = float(m_width);
	float h = float(m_height);
	float u = ps.x / w;
	float v = (h - ps.y) / h;

	float ratio = w / h;
	b2Vec2 extents(ratio * 25.0f, 25.0f);
	extents *= m_zoom;

	b2Vec2 lower = m_center - extents;
	b2Vec2 upper = m_center + extents;

	b2Vec2 pw;
	pw.x = (1.0f - u) * lower.x + u * upper.x;
	pw.y = (1.0f - v) * lower.y + v * upper.y;
	return pw;
}

//
b2Vec2 Camera::ConvertWorldToScreen(const b2Vec2& pw)
{
	float w = float(m_width);
	float h = float(m_height);
	float ratio = w / h;
	b2Vec2 extents(ratio * 25.0f, 25.0f);
	extents *= m_zoom;

	b2Vec2 lower = m_center - extents;
	b2Vec2 upper = m_center + extents;

	float u = (pw.x - lower.x) / (upper.x - lower.x);
	float v = (pw.y - lower.y) / (upper.y - lower.y);

	b2Vec2 ps;
	ps.x = u * w;
	ps.y = (1.0f - v) * h;
	return ps;
}

// Convert from world coordinates to normalized device coordinates.
// http://www.songho.ca/opengl/gl_projectionmatrix.html
void Camera::BuildProjectionMatrix(float* m, float zBias)
{
	float w = float(m_width);
	float h = float(m_height);
	float ratio = w / h;
	b2Vec2 extents(ratio * 25.0f, 25.0f);
	extents *= m_zoom;

	b2Vec2 lower = m_center - extents;
	b2Vec2 upper = m_center + extents;

	m[0] = 2.0f / (upper.x - lower.x);
	m[1] = 0.0f;
	m[2] = 0.0f;
	m[3] = 0.0f;

	m[4] = 0.0f;
	m[5] = 2.0f / (upper.y - lower.y);
	m[6] = 0.0f;
	m[7] = 0.0f;

	m[8] = 0.0f;
	m[9] = 0.0f;
	m[10] = 1.0f;
	m[11] = 0.0f;

	m[12] = -(upper.x + lower.x) / (upper.x - lower.x);
	m[13] = -(upper.y + lower.y) / (upper.y - lower.y);
	m[14] = zBias;
	m[15] = 1.0f;
}

// Prints shader compilation errors
static void sPrintLog(GLuint object)
{
	GLint log_length = 0;
	if (glIsShader(object))
		glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_length);
	else if (glIsProgram(object))
		glGetProgramiv(object, GL_INFO_LOG_LENGTH, &log_length);
	else
	{
		fprintf(stderr, "printlog: Not a shader or a program\n");
		return;
	}

	char* log = (char*)malloc(log_length);

	if (glIsShader(object))
		glGetShaderInfoLog(object, log_length, NULL, log);
	else if (glIsProgram(object))
		glGetProgramInfoLog(object, log_length, NULL, log);

	fprintf(stderr, "%s", log);
	free(log);
}

//
struct GLRenderPoints
{
	void Create()
	{
		const char* vs = \
			"#version 330\n"
			"uniform mat4 projectionMatrix;\n"
			"layout(location = 0) in vec2 v_position;\n"
			"layout(location = 1) in vec4 v_color;\n"
			"layout(location = 2) in float v_size;\n"
			"out vec4 f_color;\n"
			"void main(void)\n"
			"{\n"
			"	f_color = v_color;\n"
			"	gl_Position = projectionMatrix * vec4(v_position, 0.0f, 1.0f);\n"
			"   gl_PointSize = v_size;\n"
			"}\n";

		const char* fs = \
			"#version 330\n"
			"in vec4 f_color;\n"
			"out vec4 color;\n"
			"void main(void)\n"
			"{\n"
			"	color = f_color;\n"
			"}\n";

		NGS_NEW(shader, ngl::Shader)(vs, fs);

		vao.Active();
		vao.AddBuffer<gl::vertex_vector<ngs::float32, 2>, gl::BufferUsage::dynamic_draw>(m_vertices).Deactivate();
		vao.AddBuffer<gl::vertex_vector<ngs::float32, 4>, gl::BufferUsage::dynamic_draw>(m_colors).Deactivate();
		vao.AddBuffer<gl::vertex_vector<ngs::float32, 1>, gl::BufferUsage::dynamic_draw>(m_sizes).Deactivate();
		vao.Deactivate();

		m_count = 0;
	}

	void Destroy()
	{
		NGS_DELETE(shader);
	}

	void Vertex(const b2Vec2& v, const b2Color& c, float size)
	{
		if (m_count == e_maxVertices)
			Flush();

		m_vertices[m_count] = v;
		m_colors[m_count] = c;
		m_sizes[m_count] = size;
		++m_count;
	}

	void Flush()
	{
		if (m_count == 0)
			return;

		float proj[16] = { 0.0f };
		g_camera.BuildProjectionMatrix(proj, 0.0f);

		shader->Active();
		shader->SetUniform("projectionMatrix", proj);

		vao.UpdateData(m_count);

		gl::state.Enable(gl::Capabilities::program_point_size);
		vao.DrawArrays(gl::DrawMode::points, m_count);
		gl::state.Disable(gl::Capabilities::program_point_size);

		vao.Deactivate();
		shader->Deactivate();

		m_count = 0;
	}

	enum { e_maxVertices = 512 };
	b2Vec2 m_vertices[e_maxVertices];
	b2Color m_colors[e_maxVertices];
	float m_sizes[e_maxVertices];

	int32 m_count;

	ngl::Shader* shader = nullptr;
	ngl::VertexArray vao{};
};

//
struct GLRenderLines
{
	void Create()
	{
		const char* vs = \
			"#version 330\n"
			"uniform mat4 projectionMatrix;\n"
			"layout(location = 0) in vec2 v_position;\n"
			"layout(location = 1) in vec4 v_color;\n"
			"out vec4 f_color;\n"
			"void main(void)\n"
			"{\n"
			"	f_color = v_color;\n"
			"	gl_Position = projectionMatrix * vec4(v_position, 0.0f, 1.0f);\n"
			"}\n";

		const char* fs = \
			"#version 330\n"
			"in vec4 f_color;\n"
			"out vec4 color;\n"
			"void main(void)\n"
			"{\n"
			"	color = f_color;\n"
			"}\n";

		NGS_NEW(shader, ngl::Shader)(vs, fs);

		vao.Active();
		vao.AddBuffer<gl::vertex_vector<ngs::float32, 2>, gl::BufferUsage::dynamic_draw>(m_vertices).Deactivate();
		vao.AddBuffer<gl::vertex_vector<ngs::float32, 4>, gl::BufferUsage::dynamic_draw>(m_colors).Deactivate();
		vao.Deactivate();

		m_count = 0;
	}

	void Destroy()
	{
		NGS_DELETE(shader);
	}

	void Vertex(const b2Vec2& v, const b2Color& c)
	{
		if (m_count == e_maxVertices)
			Flush();

		m_vertices[m_count] = v;
		m_colors[m_count] = c;
		++m_count;
	}

	void Flush()
	{
		if (m_count == 0)
			return;

		shader->Active();

		float proj[16] = { 0.0f };
		g_camera.BuildProjectionMatrix(proj, 0.1f);

		shader->SetUniform("projectionMatrix", proj);
		vao.Active();
		vao.UpdateData(m_count);

		vao.DrawArrays(gl::DrawMode::lines, m_count);
		vao.Deactivate();
		shader->Deactivate();

		m_count = 0;
	}

	enum { e_maxVertices = 2 * 512 };
	b2Vec2 m_vertices[e_maxVertices];
	b2Color m_colors[e_maxVertices];

	int32 m_count;

	ngl::Shader* shader = nullptr;
	ngl::VertexArray vao{};
};

//
struct GLRenderTriangles
{
	void Create()
	{
		const char* vs = \
			"#version 330\n"
			"uniform mat4 projectionMatrix;\n"
			"layout(location = 0) in vec2 v_position;\n"
			"layout(location = 1) in vec4 v_color;\n"
			"out vec4 f_color;\n"
			"void main(void)\n"
			"{\n"
			"	f_color = v_color;\n"
			"	gl_Position = projectionMatrix * vec4(v_position, 0.0f, 1.0f);\n"
			"}\n";

		const char* fs = \
			"#version 330\n"
			"in vec4 f_color;\n"
			"out vec4 color;\n"
			"void main(void)\n"
			"{\n"
			"	color = f_color;\n"
			"}\n";

		NGS_NEW(shader, ngl::Shader)(vs, fs);

		vao.Active();
		vao.AddBuffer<gl::vertex_vector<ngs::float32, 2>, gl::BufferUsage::dynamic_draw>(m_vertices).Deactivate();
		vao.AddBuffer<gl::vertex_vector<ngs::float32, 4>, gl::BufferUsage::dynamic_draw>(m_colors).Deactivate();
		vao.Deactivate();

		m_count = 0;
	}

	void Destroy()
	{
		NGS_DELETE(shader);
	}

	void Vertex(const b2Vec2& v, const b2Color& c)
	{
		if (m_count == e_maxVertices)
			Flush();

		m_vertices[m_count] = v;
		m_colors[m_count] = c;
		++m_count;
	}

	void Flush()
	{
		if (m_count == 0)
			return;

		shader->Active();

		float proj[16] = { 0.0f };
		g_camera.BuildProjectionMatrix(proj, 0.2f);

		shader->SetUniform("projectionMatrix", proj);

		vao.Active();
		vao.UpdateData(m_count);

		gl::state.Enable(gl::Capabilities::blend);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		vao.DrawArrays(gl::DrawMode::triangles, m_count);
		gl::state.Disable(gl::Capabilities::blend);

		vao.Deactivate();
		shader->Deactivate();

		m_count = 0;
	}

	enum { e_maxVertices = 3 * 512 };
	b2Vec2 m_vertices[e_maxVertices];
	b2Color m_colors[e_maxVertices];

	int32 m_count;

	ngl::Shader* shader = nullptr;
	ngl::VertexArray vao{};
};

//
DebugDraw::DebugDraw()
{
	m_showUI = true;
	m_points = NULL;
	m_lines = NULL;
	m_triangles = NULL;
}

//
DebugDraw::~DebugDraw()
{
	b2Assert(m_points == NULL);
	b2Assert(m_lines == NULL);
	b2Assert(m_triangles == NULL);
}

//
void DebugDraw::Create()
{
	m_points = new GLRenderPoints;
	m_points->Create();
	m_lines = new GLRenderLines;
	m_lines->Create();
	m_triangles = new GLRenderTriangles;
	m_triangles->Create();
}

//
void DebugDraw::Destroy()
{
	m_points->Destroy();
	delete m_points;
	m_points = NULL;

	m_lines->Destroy();
	delete m_lines;
	m_lines = NULL;

	m_triangles->Destroy();
	delete m_triangles;
	m_triangles = NULL;
}

//
void DebugDraw::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	b2Vec2 p1 = vertices[vertexCount - 1];
	for (int32 i = 0; i < vertexCount; ++i)
	{
		b2Vec2 p2 = vertices[i];
		m_lines->Vertex(p1, color);
		m_lines->Vertex(p2, color);
		p1 = p2;
	}
}

//
void DebugDraw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	b2Color fillColor(0.5f * color.r, 0.5f * color.g, 0.5f * color.b, 0.5f);

	for (int32 i = 1; i < vertexCount - 1; ++i)
	{
		m_triangles->Vertex(vertices[0], fillColor);
		m_triangles->Vertex(vertices[i], fillColor);
		m_triangles->Vertex(vertices[i + 1], fillColor);
	}

	b2Vec2 p1 = vertices[vertexCount - 1];
	for (int32 i = 0; i < vertexCount; ++i)
	{
		b2Vec2 p2 = vertices[i];
		m_lines->Vertex(p1, color);
		m_lines->Vertex(p2, color);
		p1 = p2;
	}
}

//
void DebugDraw::DrawCircle(const b2Vec2& center, float radius, const b2Color& color)
{
	const float k_segments = 16.0f;
	const float k_increment = 2.0f * b2_pi / k_segments;
	float sinInc = sinf(k_increment);
	float cosInc = cosf(k_increment);
	b2Vec2 r1(1.0f, 0.0f);
	b2Vec2 v1 = center + radius * r1;
	for (int32 i = 0; i < k_segments; ++i)
	{
		// Perform rotation to avoid additional trigonometry.
		b2Vec2 r2;
		r2.x = cosInc * r1.x - sinInc * r1.y;
		r2.y = sinInc * r1.x + cosInc * r1.y;
		b2Vec2 v2 = center + radius * r2;
		m_lines->Vertex(v1, color);
		m_lines->Vertex(v2, color);
		r1 = r2;
		v1 = v2;
	}
}

//
void DebugDraw::DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color)
{
	const float k_segments = 16.0f;
	const float k_increment = 2.0f * b2_pi / k_segments;
	float sinInc = sinf(k_increment);
	float cosInc = cosf(k_increment);
	b2Vec2 v0 = center;
	b2Vec2 r1(cosInc, sinInc);
	b2Vec2 v1 = center + radius * r1;
	b2Color fillColor(0.5f * color.r, 0.5f * color.g, 0.5f * color.b, 0.5f);
	for (int32 i = 0; i < k_segments; ++i)
	{
		// Perform rotation to avoid additional trigonometry.
		b2Vec2 r2;
		r2.x = cosInc * r1.x - sinInc * r1.y;
		r2.y = sinInc * r1.x + cosInc * r1.y;
		b2Vec2 v2 = center + radius * r2;
		m_triangles->Vertex(v0, fillColor);
		m_triangles->Vertex(v1, fillColor);
		m_triangles->Vertex(v2, fillColor);
		r1 = r2;
		v1 = v2;
	}

	r1.Set(1.0f, 0.0f);
	v1 = center + radius * r1;
	for (int32 i = 0; i < k_segments; ++i)
	{
		b2Vec2 r2;
		r2.x = cosInc * r1.x - sinInc * r1.y;
		r2.y = sinInc * r1.x + cosInc * r1.y;
		b2Vec2 v2 = center + radius * r2;
		m_lines->Vertex(v1, color);
		m_lines->Vertex(v2, color);
		r1 = r2;
		v1 = v2;
	}

	// Draw a line fixed in the circle to animate rotation.
	b2Vec2 p = center + radius * axis;
	m_lines->Vertex(center, color);
	m_lines->Vertex(p, color);
}

//
void DebugDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
{
	m_lines->Vertex(p1, color);
	m_lines->Vertex(p2, color);
}

//
void DebugDraw::DrawTransform(const b2Transform& xf)
{
	const float k_axisScale = 0.4f;
	b2Color red(1.0f, 0.0f, 0.0f);
	b2Color green(0.0f, 1.0f, 0.0f);
	b2Vec2 p1 = xf.p, p2;

	m_lines->Vertex(p1, red);
	p2 = p1 + k_axisScale * xf.q.GetXAxis();
	m_lines->Vertex(p2, red);

	m_lines->Vertex(p1, green);
	p2 = p1 + k_axisScale * xf.q.GetYAxis();
	m_lines->Vertex(p2, green);
}

//
void DebugDraw::DrawPoint(const b2Vec2& p, float size, const b2Color& color)
{
	m_points->Vertex(p, color, size);
}

//
void DebugDraw::DrawString(int x, int y, const char* string, ...)
{
	if (m_showUI == false)
	{
		return;
	}

	va_list arg;
	va_start(arg, string);
	ImGui::Begin("Overlay", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
	ImGui::SetCursorPos(ImVec2(float(x), float(y)));
	ImGui::TextColoredV(ImColor(230, 153, 153, 255), string, arg);
	ImGui::End();
	va_end(arg);
}

//
void DebugDraw::DrawString(const b2Vec2& pw, const char* string, ...)
{
	b2Vec2 ps = g_camera.ConvertWorldToScreen(pw);

	va_list arg;
	va_start(arg, string);
	ImGui::Begin("Overlay", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
	ImGui::SetCursorPos(ImVec2(ps.x, ps.y));
	ImGui::TextColoredV(ImColor(230, 153, 153, 255), string, arg);
	ImGui::End();
	va_end(arg);
}

//
void DebugDraw::DrawAABB(b2AABB* aabb, const b2Color& c)
{
	b2Vec2 p1 = aabb->lowerBound;
	b2Vec2 p2 = b2Vec2(aabb->upperBound.x, aabb->lowerBound.y);
	b2Vec2 p3 = aabb->upperBound;
	b2Vec2 p4 = b2Vec2(aabb->lowerBound.x, aabb->upperBound.y);

	m_lines->Vertex(p1, c);
	m_lines->Vertex(p2, c);

	m_lines->Vertex(p2, c);
	m_lines->Vertex(p3, c);

	m_lines->Vertex(p3, c);
	m_lines->Vertex(p4, c);

	m_lines->Vertex(p4, c);
	m_lines->Vertex(p1, c);
}

//
void DebugDraw::Flush()
{
	m_triangles->Flush();
	m_lines->Flush();
	m_points->Flush();
}
