from PIL import Image, ImageDraw
import os

def make_hard_transparent_and_ico(source_path, png_path, ico_path):
    # Load source
    img = Image.open(source_path).convert("RGBA")
    width, height = img.size
    
    # Create circular mask
    # We use a slightly smaller radius to cut off the outer "glow" pixels 
    # that cause the black ring appearance.
    mask = Image.new("L", (width, height), 0)
    draw = ImageDraw.Draw(mask)
    
    # We cut exactly at 4% margin, effectively "hardening" the edge
    # of the glowing orb to remove all outer artifacts.
    margin = int(width * 0.04)
    draw.ellipse((margin, margin, width - margin, height - margin), fill=255)
    
    # Apply mask
    result = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    result.paste(img, (0, 0), mask=mask)
    
    # Save PNG
    result.save(png_path, "PNG")
    print(f"Saved HARD-TRANSPARENT PNG to {png_path}")
    
    # Standard Windows ICO sizes with full alpha channel
    sizes = [(16, 16), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)]
    result.save(ico_path, format='ICO', sizes=sizes)
    print(f"Saved multi-resolution ICO to {ico_path}")

if __name__ == "__main__":
    src = "/Users/kierenpatel/.gemini/antigravity/brain/f53cffab-f06d-4312-bc77-9f7aa62de87e/betterangle_simple_cyan_icon_transparent_1776142199249.png"
    assets_dir = "/Users/kierenpatel/.gemini/antigravity/scratch/BetterAngle/assets"
    
    png_out = os.path.join(assets_dir, "logo.png")
    ico_out = os.path.join(assets_dir, "icon.ico")
    
    make_hard_transparent_and_ico(src, png_out, ico_out)
