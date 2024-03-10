#include "pch.h"
#include "debug_draw_interface.h"

#include <algorithm>
#include <array>

void debug_draw_interface::resize(uint32_t new_width, uint32_t new_height)
{
	//check if screen already that size
	if (screen_width == new_width && screen_height == new_height)
	{
		return;
	}

	screen_pixles.resize(new_width * new_height);

	screen_width = new_width;
	screen_height = new_height;
}

void debug_draw_interface::clear_to(colour_type colour)
{
	std::for_each(screen_pixles.begin(), screen_pixles.end(), [&](auto& pixel_colour) {pixel_colour = colour; });
}


void debug_draw_interface::draw_screen_space_line(math_2d_util::ivec2d from, math_2d_util::ivec2d to, colour_type colour)
{
    // Calculate differences in coordinates
    math_2d_util::ivec2d dif(abs(to.x - from.x), abs(to.y - from.y));

    // Determine the direction of movement along x and y axes
    math_2d_util::ivec2d draw_dir((from.x < to.x) ? 1 : -1, (from.y < to.y) ? 1 : -1);

    // Initialize the error term
    int err = dif.x - dif.y;

    for(int i = 0 ; i < screen_pixles.size(); i++) 
    {
        // Draw the pixel at (x1, y1)
       
        //check that pixel is on screen
        if (from.x >= 0 && from.x < screen_width && from.y >= 0 && from.y < screen_height)
        {
            //calcualte the tile 
            uint32_t tile = (screen_width * from.y) + from.x;

            //set pixel colour
            screen_pixles[tile] = colour;
        }

        // Check if the endpoint is reached
        if (from == to) 
        {
            break;
        }

        // Calculate error term for the next iteration
        int err2 = 2 * err;

        // Adjust x coordinate and update error term
        if (err2 > -dif.y) 
        {
            err -= dif.y;
            from.x += draw_dir.x;
        }

        // Adjust y coordinate and update error term
        if (err2 < dif.x) 
        {
            err += dif.x;
            from.y += draw_dir.y;
        }
    }
}

void debug_draw_interface::draw_dotted_screen_space_line(math_2d_util::ivec2d from, math_2d_util::ivec2d to, colour_type colour, uint32_t pixels_per_dot, uint32_t pixels_per_space)
{
    if (pixels_per_space == 0)
    {
        pixels_per_space = pixels_per_dot;
    }

    // Calculate differences in coordinates
    math_2d_util::ivec2d dif(abs(to.x - from.x), abs(to.y - from.y));

    // Determine the direction of movement along x and y axes
    math_2d_util::ivec2d draw_dir((from.x < to.x) ? 1 : -1, (from.y < to.y) ? 1 : -1);

    // Initialize the error term
    int err = dif.x - dif.y;

    int pixles_in_mode = pixels_per_dot;

    bool draw_mode = true;

    for (int i = 0; i < screen_pixles.size(); i++)
    {
        // Draw the pixel at (x1, y1)

        //check that pixel is on screen
        if (draw_mode && from.x >= 0 && from.x < screen_width && from.y >= 0 && from.y < screen_height)
        {
            //calcualte the tile 
            uint32_t tile = (screen_width * from.y) + from.x;

            //set pixel colour
            screen_pixles[tile] = colour;
        }

        --pixles_in_mode;

        if (pixles_in_mode == 0)
        {
            draw_mode = !draw_mode;

            pixles_in_mode = draw_mode ? pixels_per_space : pixels_per_dot;
        }

        // Check if the endpoint is reached
        if (from == to)
        {
            break;
        }

        // Calculate error term for the next iteration
        int err2 = 2 * err;

        // Adjust x coordinate and update error term
        if (err2 > -dif.y)
        {
            err -= dif.y;
            from.x += draw_dir.x;
        }

        // Adjust y coordinate and update error term
        if (err2 < dif.x)
        {
            err += dif.x;
            from.y += draw_dir.y;
        }
    }
}


void debug_draw_interface::draw_screen_space_box(math_2d_util::ivec2d min, math_2d_util::ivec2d max, colour_type colour)
{
    max.x += 1;
    max.y += 1;

    min = math_2d_util::ivec2d(std::max(min.x, 0), std::max(min.y, 0));

    max = math_2d_util::ivec2d(std::min(max.x, static_cast<int32_t>(screen_width -1)), std::min(max.y, static_cast<int32_t>(screen_height -1)));


    for (int iy = min.y; iy < max.y; ++iy)
    {
        for (int ix = min.x; ix < max.x; ++ix)
        {
            //calcualte the tile 
            uint32_t tile = (screen_width * iy) + ix;

            //set pixel colour
            screen_pixles[tile] = colour;
        }
    }
}

void debug_draw_interface::draw_screen_space_box_outline(math_2d_util::ivec2d min, math_2d_util::ivec2d max, colour_type colour)
{
    draw_screen_space_box_outline_internal(min, max, colour, [&](auto min, auto max, auto colour) { draw_screen_space_line(min, max, colour); });
}

void debug_draw_interface::draw_screen_space_circle(math_2d_util::ivec2d center, int32_t radius, colour_type colour)
{

    math_2d_util::ivec2d min = math_2d_util::ivec2d(std::max(center.x - radius, 0), std::max(center.y - radius, 0));

    math_2d_util::ivec2d max = math_2d_util::ivec2d(std::min(center.x + radius + 1, static_cast<int32_t>(screen_width)), std::min(center.y + radius + 1, static_cast<int32_t>(screen_height)));


    for (int iy = min.y; iy < max.y; ++iy)
    {
        for (int ix = min.x; ix < max.x; ++ix)
        {
            //check if in or our of circle
            math_2d_util::ivec2d dif = math_2d_util::ivec2d(ix, iy) - center;

            //get squared length
            if (dif.lenght_sqr() > (radius * radius))
            {
                //skip if not in circle
                continue;
            }

            //calcualte the tile 
            uint32_t tile = (screen_width * iy) + ix;

            //set pixel colour
            screen_pixles[tile] = colour;
        }
    }
}

void debug_draw_interface::draw_sceen_space_circle_outline(math_2d_util::ivec2d center, int32_t radius, colour_type colour)
{
    int x = radius;
    int y = 0;
    int radiusError = 1 - x;

    while (x >= y) 
    {

        std::array<math_2d_util::ivec2d, 8> tile;
       
        // Set the outline points in all octants
         tile[0] =math_2d_util::ivec2d( (center.x + x),(center.y + y));
         tile[1] =math_2d_util::ivec2d( (center.x - x),(center.y + y));
         tile[2] =math_2d_util::ivec2d( (center.x + x),(center.y - y));
         tile[3] =math_2d_util::ivec2d( (center.x - x),(center.y - y));
         tile[4] =math_2d_util::ivec2d( (center.x + y),(center.y + x));
         tile[5] =math_2d_util::ivec2d( (center.x - y),(center.y + x));
         tile[6] =math_2d_util::ivec2d( (center.x + y),(center.y - x));
         tile[7] =math_2d_util::ivec2d( (center.x - y),(center.y - x));

         for(auto tile_index : tile)
         {
             if (tile_index.x >= 0 && tile_index.x < screen_width &&
                 tile_index.y > 0 && tile_index.y < screen_height)
             {
                 screen_pixles[tile_index.x + (tile_index.y * screen_width)] = colour;
             }
         }

        y++;

        if (radiusError < 0) {
            radiusError += 2 * y + 1;
        }
        else {
            x--;
            radiusError += 2 * (y - x + 1);
        }
    }
}

void debug_draw_interface::draw_screen_space_box_outline_internal(math_2d_util::ivec2d min, math_2d_util::ivec2d max, colour_type colour, std::function<void(math_2d_util::ivec2d, math_2d_util::ivec2d, colour_type colour)> line_draw_func)
{
    //draw top
    line_draw_func(math_2d_util::ivec2d(min.x, min.y), math_2d_util::ivec2d(max.x, min.y), colour);
    line_draw_func(math_2d_util::ivec2d(min.x, max.y), math_2d_util::ivec2d(max.x, max.y), colour);

    line_draw_func(math_2d_util::ivec2d(min.x, min.y), math_2d_util::ivec2d(min.x, max.y), colour);
    line_draw_func(math_2d_util::ivec2d(max.x, min.y), math_2d_util::ivec2d(max.x, max.y), colour);
}

